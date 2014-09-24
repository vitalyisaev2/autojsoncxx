#!/usr/bin/env python
# -*- coding: utf-8 -*-

# The MIT License (MIT)
#
# Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import unicode_literals
from __future__ import print_function

import re
import argparse
import os
import hashlib
import xml.etree.ElementTree as ET
import io

open = io.open

import clang
import clang.cindex


class AnnotationError(Exception):
    def __init__(self, node):
        self.node = node

    def __str__(self):
        return "The annotation syntax is incorrect: " + repr(self.node)


def get_full_class_name(class_declaration):
    name = '::' + class_declaration.displayname
    cursor = class_declaration
    while cursor.semantic_parent and cursor.semantic_parent.kind == clang.cindex.CursorKind.NAMESPACE:
        name = '::' + cursor.semantic_parent.displayname + name
        cursor = cursor.semantic_parent
    return name


def get_full_type_name(clang_type):
    clang_type = clang_type.get_canonical()

    if not hasattr(get_full_type_name, 'mapping'):
        get_full_type_name.mapping = {clang.cindex.TypeKind.BOOL: "bool",
                                      clang.cindex.TypeKind.NULLPTR: "::std::nullptr_t",
                                      clang.cindex.TypeKind.CHAR_U: "char",
                                      clang.cindex.TypeKind.CHAR_S: "char",
                                      clang.cindex.TypeKind.INT: "int",
                                      clang.cindex.TypeKind.UINT: "unsigned int",
                                      clang.cindex.TypeKind.LONG: "long",
                                      clang.cindex.TypeKind.ULONG: "unsigned long",
                                      clang.cindex.TypeKind.LONGLONG: "long long",
                                      clang.cindex.TypeKind.ULONGLONG: "unsigned long long",
                                      clang.cindex.TypeKind.DOUBLE: "double"}

    try:
        return get_full_type_name.mapping[clang_type.kind]
    except KeyError:
        if clang_type.kind == clang.cindex.TypeKind.RECORD:
            return get_full_class_name(clang_type.get_declaration())
        raise


def hard_escape(text):
    def escape(char):
        return '\\x{:02x}'.format(ord(char))

    return '"' + ''.join(escape(char) for char in text) + '"'


def get_truth_value(element_node):
    if element_node is None:
        return False
    if element_node.text.lower() == 'true':
        return True
    if element_node.text.lower() == 'false':
        return False

    raise AnnotationError(element_node)


class ClassInfo:
    def __init__(self):
        self.name = ''
        self.strict_parsing = False
        self.fields = []

    def parse_annotation(self, annotation):
        if not annotation:
            return

        node = ET.fromstring(annotation)
        if node.tag != 'codegen':
            raise AnnotationError(node)
        self.strict_parsing = get_truth_value(node.find('strict_parsing'))


class FieldInfo:
    def __init__(self):
        self.type_name = ''
        self.variable_name = ''
        self.required = False
        self.ignore = ''
        self.json_key = ''

    def parse_annotation(self, annotation):
        if not annotation:
            return

        node = ET.fromstring(annotation)
        if node.tag != 'codegen':
            raise AnnotationError(node)

        require_node = node.find('required')
        self.required = get_truth_value(require_node)

        ignore_node = node.find('ignore')
        self.ignore = get_truth_value(ignore_node)

        json_key_node = node.find('key')
        self.json_key = json_key_node.text if json_key_node is not None else self.variable_name

    def generate_flag_statement(self, flag):
        if self.required:
            return 'has_{} = {};'.format(self.variable_name, flag)
        else:
            return ''


def extract_field_information(cursor):
    for c in cursor.get_children():
        if c.kind == clang.cindex.CursorKind.FIELD_DECL:
            info = FieldInfo()
            info.type_name = get_full_type_name(c.type)
            info.variable_name = c.displayname
            info.json_key = info.variable_name

            for cc in c.get_children():
                if cc.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                    info.parse_annotation(cc.displayname)

            yield info


def extract_class_information(cursor, _filter):
    for c in cursor.get_children():
        if not _filter(c):
            continue

        if c.kind == clang.cindex.CursorKind.NAMESPACE:
            for cc in extract_class_information(c, _filter):
                yield cc
        elif c.kind == clang.cindex.CursorKind.CLASS_DECL or c.kind == clang.cindex.CursorKind.STRUCT_DECL:
            info = ClassInfo()
            info.name = get_full_class_name(c)
            info.fields = list(extract_field_information(c))

            for cc in c.get_children():
                if cc.kind == clang.cindex.CursorKind.ANNOTATE_ATTR:
                    info.parse_annotation(cc.displayname)

            yield info


class MainCodeGenerator:
    def __init__(self, class_info):
        self.class_info = class_info
        self.fields_info = [f for f in class_info.fields if not f.ignore]

    def handler_declarations(self):
        return '\n'.join('SAXEventHandler< {} > handler_{};'.format(m.type_name, i)
                         for i, m in enumerate(self.fields_info))

    def handler_initializers(self):
        return '\n'.join(', handler_{}(&obj->{})'.format(i, m.variable_name)
                         for i, m in enumerate(self.fields_info))

    def flags_declaration(self):
        return '\n'.join('bool has_{};'.format(m.variable_name) for m in self.fields_info if m.required)

    def flags_reset(self):
        return '\n'.join(m.generate_flag_statement("false") for m in self.fields_info)

    def post_validation(self):
        return '\n'.join('if (!has_{0}) set_missing_required("{0}");'
                             .format(m.variable_name) for m in self.fields_info if m.required)

    def key_event_handling(self):
        return '\n'.join('else if (utility03B1B951445A::string_equal(str, length, {key}, {key_length}))\n\
                         {{ state={state}; {check} }}'
                             .format(key=hard_escape(m.json_key), key_length=len(m.json_key),
                                     state=i, check=m.generate_flag_statement("true"))
                         for i, m in enumerate(self.fields_info))

    def event_forwarding(self, call_text):
        return '\n\n'.join('case {i}:\n    return checked_event_forwarding(handler_{i}.{call});'
                               .format(i=i, call=call_text) for i in range(len(self.fields_info)))

    def error_reaping(self):
        return '\n'.join('case {0}:\n     handler_{0}.ReapError(errs); break;'.format(i)
                         for i in range(len(self.fields_info)))

    def writer_type_name(self):
        return "Writer" + hashlib.sha256(self.class_info.name.encode('utf-8')).hexdigest()

    def data_serialization(self):
        return '\n'.join('w.Key({}); Serializer< {}, {} >()(w, value.{});'
                             .format(hard_escape(m.json_key), self.writer_type_name(),
                                     m.type_name, m.variable_name)
                         for m in self.fields_info)

    def current_member_name(self):
        return '\n'.join('case {}:\n    return "{}";'.format(i, m.variable_name)
                         for i, m in enumerate(self.fields_info))

    def unknown_key_handling(self):
        if self.class_info.strict_parsing:
            return 'the_error.reset(new error::UnknownFieldError(str, length)); return false;'
        else:
            return 'return true;'


def build_class(template, class_info):
    gen = MainCodeGenerator(class_info)

    replacement = {
        "list of declarations": gen.handler_declarations() + gen.flags_declaration(),
        "init": gen.handler_initializers(),
        "serialize all members": gen.data_serialization(),
        "change state": gen.key_event_handling(),
        "reap error": gen.error_reaping(),
        "get member name": gen.current_member_name(),
        "validation": gen.post_validation(),
        "reset flags": gen.flags_reset(),
        "handle unknown key": gen.unknown_key_handling(),
        "TypeName": class_info.name,
        "Writer": gen.writer_type_name()}

    def evaluate(match):
        try:
            return replacement[match.group(1)]
        except KeyError:
            match = re.match(r'forward (.*?) to members', match.group(1))
            if match:
                return gen.event_forwarding(match.group(1))
            else:
                raise

    return re.sub(r'/\*\s*(.*?)\s*\*/', evaluate, template)


def main():
    parser = argparse.ArgumentParser(description='`autojsoncxx` code generator '
                                                 '(visit https://github.com/netheril96/autojsoncxx for details)')

    parser.add_argument('-i', '--input', help='input file for the header or source of class definitions', required=True)
    parser.add_argument('-o', '--output', help='output file name for the generated header file', required=True)
    parser.add_argument('--args', action='append',
                        help='arguments passed to clang (e.g. "--args=-I../include --args=--std=c++11")')
    parser.add_argument('--clang', help='directory where libclang resides in')
    parser.add_argument('--template', help='location of the template file', default=None)
    args = parser.parse_args()

    if args.template is None:
        args.template = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'code_template')

    if args.clang is None:
        import platform

        system = platform.system().lower()
        if system == 'darwin':
            args.clang = '/Library/Developer/CommandLineTools/usr/lib'
        elif os.path.exists('/usr/lib/libclang.so'):
            args.clang = '/usr/lib/'
        elif os.path.exists('/usr/local/lib/libclang.so'):
            args.clang = '/usr/local/lib'

    if args.clang is not None:
        clang.cindex.Config.set_library_path(args.clang)

    index = clang.cindex.Index.create()
    translation_unit = index.parse(args.input, ['-x', 'c++'] + args.args)
    cursor = translation_unit.cursor

    with open(args.template) as f:
        template = f.read()
        with open(args.output, 'w') as out:
            def _filter(c):
                return c.location.file.name == args.input

            for class_info in extract_class_information(cursor, _filter):
                out.write(build_class(template, class_info))
                out.write('\n')


if __name__ == '__main__':
    main()