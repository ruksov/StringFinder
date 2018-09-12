# python script for checking result output log for StringFinder
import argparse
     
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("haystack_path", type=str,  help="path to haystack file")
    parser.add_argument("out_path", type=str)
    parser.add_argument("hs_offset", type=int, help="hs_offset")
    parser.add_argument("hs_len", type=int, help="hs_len")
    args = parser.parse_args()
    
    # open file
    hs_file = open(args.haystack_path, "rb")
    
    out = open(args.out_path, 'wb')
    hs_file.seek(args.hs_offset)
    hs_data = hs_file.read(args.hs_len)
    out.write(hs_data)