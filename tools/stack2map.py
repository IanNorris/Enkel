import re, sys, argparse
from collections import defaultdict

def load_ld_map(file_path):
    pattern_file = re.compile(r"^\s*(?:\.\w+\s+)?0x([0-9A-Fa-f]+)\s+0x([0-9A-Fa-f]+)\s+(.*)$")
    pattern = re.compile(r"^\s*0x([0-9A-Fa-f]+)\s+(.*)$")
    map_data = []
    
    with open(file_path, 'r') as f:
        lines = f.readlines()

    for line in lines:
        match = pattern.match(line)
        if match:
            match_file = pattern_file.match(line)
            if match_file:
                continue
            address = int(match.group(1), 16)
            name = match.group(2)
    
            if not name:
                continue
            
            map_data.append({
                'address': address,
                'name': name
            })

    map_data.sort(key=lambda x: x['address'])

    return map_data

def find_last_entry_before_offset(map_data, offset):
    # Assumes map_data is sorted by address
    for i in reversed(range(len(map_data))):
        if map_data[i]['address'] <= offset:
            return map_data[i]
    return None

def process_input_file(map_data, input_file_path):
    with open(input_file_path, 'r') as f:
        lines = f.readlines()

    for line in lines:
        if line.startswith("#"):
            tokens = line[1:].split(":")
            if len(tokens) == 2:
                index = tokens[0].strip()
                offset = int(tokens[1].strip(), 16)

                matching_entry = find_last_entry_before_offset(map_data, offset)
                
                if matching_entry:
                    name = matching_entry['name']
                    offset_diff = offset - matching_entry['address']
                    print(f"#0x{index}: 0x{offset:x} ({name}+0x{offset_diff:x})")
                else:
                    print(f"#0x{index}: 0x{offset:x} (Unknown)")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process an LD map file and an input file to map function addresses.')
    
    parser.add_argument('-m', '--map', required=True, help='Path to the LD map file')
    parser.add_argument('-i', '--input', required=True, help='Path to the input file')
    parser.add_argument('-o', '--output', help='Path to the output file (Optional)')
    
    args = parser.parse_args()
    ld_map_file_path = args.map
    input_file_path = args.input
    output_file_path = args.output

    map_data = load_ld_map(ld_map_file_path)
    
    if output_file_path:
        with open(output_file_path, 'w') as f:
            original_stdout = sys.stdout
            sys.stdout = f
            process_input_file(map_data, input_file_path)
            sys.stdout = original_stdout
    else:
        process_input_file(map_data, input_file_path)
