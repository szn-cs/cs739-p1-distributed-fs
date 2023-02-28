#! /usr/bin/env python3

import os
import fs_util
import sys
import test2_clientA_largefile as test2  # Simply do not want to do test2_common
'''
This is ClientB.
'''


def run_test():
    signal_name_gen = fs_util.get_fs_signal_name()

    cur_signal_name = next(signal_name_gen)
    fs_util.record_test_result(test2.TEST_CASE_NO, 'B',
                               f'START fname:{test2.FNAME}')
    fs_util.wait_for_signal(cur_signal_name)

    # first execution, read all-zero file
    if not fs_util.path_exists(test2.FNAME):
        fs_util.record_test_result(test2.TEST_CASE_NO, 'B', 'not exist')
        sys.exit(1)
    fd = fs_util.open_file(test2.FNAME)
    read_len = 32768
    read_str = fs_util.read_file(fd, read_len, 0)
    if len(read_str) != read_len:
        fs_util.record_test_result(test2.TEST_CASE_NO, 'B',
                                   f'read_len:{len(read_str)}')
        sys.exit(1)
    for rc in read_str:
        if rc != '0':
            fs_util.record_test_result(test2.TEST_CASE_NO, 'B',
                                       f'read_str:{read_str}')
            sys.exit(1)
    
    # ---------------- new ------------------
    # client B write 4G file
    large_file_str = fs_util.gen_str_by_repeat('B', 4000000000)
    fs_util.write_file(fd, large_file_str)
    fs_util.record_test_result(test2.TEST_CASE_NO, 'B',
                               f'Finish Read and Write of b')
    
    # before flush the large file, trigger client A sleep and close file
    last_signal_name = cur_signal_name
    cur_signal_name = next(signal_name_gen)
    
    # remove signal, let client a start to flush
    os.remove(last_signal_name)
    
    # flush large data into server
    fs_util.close_file(fd)



    fs_util.wait_for_signal(cur_signal_name)
    # should read client A write
    fd = fs_util.open_file(test2.FNAME)
    read_str = fs_util.read_file(fd, 40000, 0)
    if len(read_str) != read_len:
        fs_util.record_test_result(test2.TEST_CASE_NO, 'B',
                                   f'read_len:{len(read_str)}')
        sys.exit(1)
    for rc in read_str:
        if rc != 'A':
            fs_util.record_test_result(test2.TEST_CASE_NO, 'B',
                                       f'read_str:{read_str}')
            sys.exit(1)

    # done
    fs_util.record_test_result(test2.TEST_CASE_NO, 'B', 'OK')


if __name__ == '__main__':
    run_test()
