# python script for checking result output log for StringFinder
import argparse
     
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("result_path", type=str,  help="path to result log file")
    parser.add_argument("needle_path", type=str,  help="path to needle file")
    parser.add_argument("haystack_path", type=str,  help="path to haystack file")
    args = parser.parse_args()
    
    # open files
    res_file = open(args.result_path)
    nl_file = open(args.needle_path, "rb")
    hs_file = open(args.haystack_path, "rb")
    
    # read by line all result log file
    res_lines = res_file.readlines()
    res_lines = [x.strip() for x in res_lines]
    
    is_valid_res = True
    for index, line in enumerate(res_lines):
        # parse match length
        begin = line.index('=') + 2
        end = line.index('found')
        match_len = int(line[begin:end])

        # parse haystack offset
        begin = line.find('offset') + 7
        end = line.find(',')
        hs_offset = int(line[begin:end])
        
        # parse needle offset
        begin = line.find('offset', begin) + 7
        nl_offset = int(line[begin:])
        
        # read and compare data from files
        nl_file.seek(nl_offset, 0)
        nl_data = nl_file.read(match_len)
        hs_file.seek(hs_offset, 0)
        hs_data = hs_file.read(match_len)
        
        if(nl_data != hs_data):
            is_valid_res = False
            print('wrong result in ' + str(index + 1) + ' line of result log')
    
    if is_valid_res:
        print('Result log is correct')