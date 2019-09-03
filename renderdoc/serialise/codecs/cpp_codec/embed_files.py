import glob
import os
import sys

def EscapeChar(c, in_quotes):
    c = chr(c)
    out = c
    if c == '\t':
        out = '\\t'
    elif c == '\n':
        out = '\\n'
    elif c == '\r':
        out = '\\r'
    elif c == '"' and in_quotes:
        out = '\\"'
    elif c == "'" and not in_quotes:
        out = "\\'"
    elif c == '\\':
        out = '\\\\'
    elif ord(c) < 32 or ord(c) > 127:
        out = '\\x%02x' % ord(c)
    return out


def WriteContentsAsString(ofile, contents, indent):
    ofile.write('\n')
    column = 0
    for c in contents:
        if column == 0:
            ofile.write(' ' * indent + '"')
        out_str = EscapeChar(c, True)
        ofile.write(out_str)
        column += len(out_str)
        if column > 120:
            column = 0
            ofile.write('"\n')
    if column > 0:
        ofile.write('"')
    ofile.write(';\n')


def WriteContentsAsBytes(ofile, contents, indent):
    ofile.write(' {\n')
    column = 0
    for c in contents:
        if column == 0:
            ofile.write(' ' * indent)
        out_str = "'%s'," % EscapeChar(c, False)
        ofile.write(out_str)
        column += len(out_str)
        if column > 120:
            column = 0
            ofile.write('\n')
    if column > 0:
        ofile.write('\n')
    ofile.write(' ' * indent + '};\n')


def WriteContents(ofile, contents, indent):
    if len(contents) > 65534:  # VS has a limit for static string size of 65535 chars (including \0)
        WriteContentsAsBytes(ofile, contents, indent)
    else:
        WriteContentsAsString(ofile, contents, indent)

def main():
    if len(sys.argv) < 4:
        print("python embed_files.py output.cpp input_dir/ file1 subdir/* ...")
        sys.exit(1)

    outfile = sys.argv[1]
    input_dir = sys.argv[2]
    input_files = sys.argv[3:]

    with open(outfile, "wt") as ofile:
        ofile.write(('#include "cpp_codec_resource.h"\n'
                     'namespace cpp_codec\n'
                     '{\n'
                     'namespace\n'
                     '{\n'))
        file_data = []
        index = 1
        for inputfile in input_files:
            for full_name in glob.glob(os.path.join(input_dir, inputfile)):
                rel_name = os.path.normpath(os.path.relpath(full_name, input_dir)).replace('\\', '/')
                if os.path.isdir(full_name):
                    input_files.append(rel_name + "/*")
                    continue
                size = os.path.getsize(full_name)
                file_data.append((rel_name, size))
                ofile.write('const char fileData_%d[] =' % index)
                WriteContents(ofile, open(full_name, "rb").read(), 2)
                index += 1
        ofile.write(('}    // namespace\n'
                     '\n'
                     'rdcarray<ResourceFileDesc> ResourceFiles = {'))
        for index, data in enumerate(file_data):
            ofile.write('  {R"(%s)", %d, fileData_%d},\n' % (data[0], data[1], index + 1))
        ofile.write('};\n')
        ofile.write('}    // namespace vk_cpp_codec\n')


if __name__ == "__main__":
    main()
