# StringFinder
Program search matches of needle file data in haystack file with threshold

Current aproximate performance is 15 minuts on needle data ~2Mb, haystack data ~8Gb and threshold 1000 bytes.
If needle data is LE 1Mb, program will use ~200Mb of memory (no metter what size of haystack).
