#! /usr/bin/env python3

import os
import fs_util
import sys
import time
'''
This is ClientA.
'''

cs739_env_vars = [
    'CS739_CLIENT_A', 'CS739_CLIENT_B', 'CS739_SERVER', 'CS739_MOUNT_POINT'
]
ENV_VARS = {var_name: os.environ.get(var_name) for var_name in cs739_env_vars}
for env_var in ENV_VARS.items():
    print(env_var)
    assert env_var is not None
TEST_DATA_DIR = ENV_VARS['CS739_MOUNT_POINT'] + '/test_consistency'
FNAME = f'{TEST_DATA_DIR}/case2'
print(TEST_DATA_DIR)
TEST_CASE_NO = 2


def run_test():
    host_b = ENV_VARS['CS739_CLIENT_B']
    assert fs_util.test_ssh_access(host_b)
    signal_name_gen = fs_util.get_fs_signal_name()

    if not fs_util.path_exists(TEST_DATA_DIR):
        fs_util.mkdir(TEST_DATA_DIR)

    # init
    if not fs_util.path_exists(FNAME):
        fs_util.create_file(FNAME)
     
    init_str = fs_util.gen_str_by_repeat('0', 32768)
    fd = fs_util.open_file(FNAME)
    fs_util.write_file(fd, init_str)
    fs_util.close_file(fd)
    
    # open again, client A write large files:
    fd = fs_util.open_file(FNAME)
    small_file_a = fs_util.gen_str_by_repeat('A', 40000)
    fs_util.write_file(fd, small_file_a)


    # time for client_b to work, host_b should read the all-zero file
    cur_signal_name = next(signal_name_gen)
    fs_util.start_another_client(host_b, 1, 'B', cur_signal_name)


    while True:
        removed = fs_util.poll_signal_remove(host_b, cur_signal_name)
        if removed:
            break
        time.sleep(0.5)
    
    fs_util.close_file(fd)

    os.wait(3)
    
    # confirm is A
    fd = fs_util.open_file(FNAME)
    
    should_failed_read = fs_util.read_file(fd, 40001)
    # python will not throw error, but it will fill the non-value data as emptry string.
    assert should_failed_read[40000] == b''

    
    cur_str = fs_util.read_file(fd, 40000)
    assert len(cur_str) == 40000
    for idx, rc in enumerate(cur_str):
        if rc != 'A':
            print(f'Error idx:{idx} rc:{rc}')
    
    last_signal_name = cur_signal_name
    cur_signal_name = next(signal_name_gen)
    fs_util.send_signal(host_b, cur_signal_name)
    # finish
    fs_util.record_test_result(TEST_CASE_NO, 'A', 'OK')



if __name__ == '__main__':
    run_test()
