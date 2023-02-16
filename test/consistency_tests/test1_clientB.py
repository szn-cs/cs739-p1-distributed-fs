#! /usr/bin/env python3

import os
import fs_util
import sys
import test1_clientA as test1  # Simply do not want to do test1_common
'''
This is ClientB.
'''


def run_test():
    signal_name_gen = fs_util.get_fs_signal_name()

    cur_signal_name = next(signal_name_gen)
    fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                               f'START fname:{test1.FNAME}')
    fs_util.wait_for_signal(cur_signal_name)

    # first execution, read all-zero file
    if not fs_util.path_exists(test1.FNAME):
        fs_util.record_test_result(test1.TEST_CASE_NO, 'B', 'not exist')
        sys.exit(1)
    fd = fs_util.open_file(test1.FNAME)
    read_len = 32768
    read_str = fs_util.read_file(fd, read_len, 0)
    if len(read_str) != read_len:
        fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                                   f'read_len:{len(read_str)}')
        sys.exit(1)
    for rc in read_str:
        if rc != '0':
            fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                                       f'read_str:{read_str}')
            sys.exit(1)

    # let's do some write
    cur_str = fs_util.gen_str_by_repeat('b', 100)
    fs_util.write_file(fd, cur_str, start_off=100)
    fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                               f'Finish Read and Write of b')
    # suppose to flush
    fs_util.close_file(fd)

    last_signal_name = cur_signal_name
    cur_signal_name = next(signal_name_gen)
    fs_util.wait_for_signal(cur_signal_name, last_signal_name=last_signal_name)

    fd = fs_util.open_file(test1.FNAME)
    cur_str = fs_util.read_file(fd, 300)
    if len(cur_str) != 300:
        fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                                   f'read_len:{len(cur_str)}')
    for idx, c in enumerate(cur_str):
        if idx < 100:
            if c != 'a':
                fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                                           f'idx:{idx} c:{c}')
                sys.exit(1)
        elif idx < 200:
            if c != 'b':
                fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                                           f'idx:{idx} c:{c}')
                sys.exit(1)
        else:
            if c != '0':
                fs_util.record_test_result(test1.TEST_CASE_NO, 'B',
                                           f'idx:{idx} c:{c}')
                sys.exit(1)
    # done
    fs_util.record_test_result(test1.TEST_CASE_NO, 'B', 'OK')


if __name__ == '__main__':
    run_test()
