import sys
import os

def main():
    if len(sys.argv) != 3:
        print('Usage: python bin2c.py input_filename output_filename_base')
        sys.exit(1)

    filename = sys.argv[1]
    target_filename = sys.argv[2]
    base_filename = os.path.splitext(os.path.basename(filename))[0].replace('.', '_')
    file_ext = os.path.splitext(filename)[1].replace('.', '_')

    # read binary data
    with open(filename, 'rb') as f:
        data = f.read()
    
    data_variable_name = base_filename + file_ext + '_data'

    # write .c file
    with open(target_filename + '.cpp', 'w') as f:
        f.write('extern "C" const unsigned char ' + data_variable_name + '[] __attribute__((used)) = {')
        f.write(','.join(f"{x:#0{4}x}" for x in data))
        f.write('};\n')
        f.write('extern "C" const unsigned int ' + base_filename + file_ext + '_size __attribute__((used)) = sizeof(' + data_variable_name + ');\n')

    # write .h file
    with open(target_filename + '.h', 'w') as f:
        f.write('extern "C" const unsigned char ' + data_variable_name + '[];\n')
        f.write('extern "C" const unsigned int ' + base_filename + file_ext + '_size;\n')

if __name__ == '__main__':
    main()
