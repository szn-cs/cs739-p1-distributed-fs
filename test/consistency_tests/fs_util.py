#! /usr/bin/env python3

import os
import math
import subprocess
import getpass
import sys
import time
import json
import paramiko
from typing import List, Tuple


def get_shell_cmd_output(
        exec_dir: str,
        cmd: str,
        timeout_sec: float = None) -> Tuple[List[str], List[str], int]:
    """get the output of executing a shell command

    Args:
        exec_dir (str): the directory where to execute the command
        cmd (str): shell command to execute
        timeout_sec (float): number of second for the timeout, default None

    Returns:
        Tuple[List[str], List[str]]: (stdout output, stderr output)
    """
    proc = subprocess.run(cmd,
                          capture_output=True,
                          shell=True,
                          cwd=exec_dir,
                          timeout=timeout_sec)
    outs = proc.stdout.decode(encoding="utf-8").splitlines()
    errs = proc.stderr.decode(encoding="utf-8").splitlines()
    return outs, errs, proc.returncode


def gen_str_by_repeat(seed_str: str, l: int, tail: str = None) -> str:
    """Generate content of files (used for write)
    E.g., 'aabbcc'  repeat to format a string of length 1000 can be done
        by gen_str_by_repeat('aabbcc', 1000)
    Args:
        seed_str (str): will repeat this seed_str until length is met
        l (int): length of the resulting string
        tail (str, optional): can have special tail. Defaults to None.
    """
    seed_len = len(seed_str)
    ret_str = (seed_str * math.ceil(l / seed_len))[0:l]
    if tail is not None and len(tail) > 0:
        ret_str = ret_str[0:l - len(tail)] + tail
    assert len(ret_str) == l
    return ret_str


def test_ssh_access(host: str) -> bool:
    ssh_cmd = f'ssh {host} "ifconfig | grep inet"'
    outs, _errs, ret = get_shell_cmd_output(None, ssh_cmd)
    for lo in outs:
        print(lo)
    return ret == 0 and len(outs) > 0


def get_fs_signal_name():
    i = 0
    while True:
        yield f'/tmp/ClientA_signal_{i}'
        i += 1


def record_test_result(case_no: int, client_id: str, msg: str):
    fname = f'/tmp/Client{client_id.upper()}_test{case_no}'
    cmd_str = f'date >> {fname}'
    os.system(cmd_str)
    cmd_str = f'echo {msg} >> {fname}'
    os.system(cmd_str)


def wait_for_signal(signal_fname: str,
                    do_print=False,
                    last_signal_name: str = ''):
    """
    Wait for a signal to keep executing.
    The signal is simply testing if a file in a predefined exist or not.
    This client first remove last signal_fname which it last time waited on
    so that the signaling client knows this client finished and started to
    wait for next signal.
    """
    if len(last_signal_name) > 0 and os.path.exists(last_signal_name):
        # tell the sender that this client finish the execution, so it can
        # go on
        os.remove(last_signal_name)
    if do_print:
        print(f'wait for signal file {signal_fname}')
    while not os.path.exists(signal_fname):
        time.sleep(0.7)


def start_another_client(host: str, test_case: int, client_id: str,
                         signal_fname: str):
    """Invoke the python script for another client via ssh.
        REQUIRED: $USERNAME/.ssh/id_rsa.pub is added to remote client's
        ~/.ssh/authorized_keys , or ensure client.connect() can succeed.
        REQUIRED: put 739p1.env into ~/

    Args:
        host (str): hostname (remote)
        test_case (int): test case number
        client_id (str): client id (remote)
        signal_fname (str): started remote client will wait for this signal.
            and after it finish execution, it will remove this signal_file,
            so this client can know it.
    """
    send_signal(host, signal_fname)
    signal_exists = (not poll_signal_remove(host, signal_fname))
    assert signal_exists
    script_name = f'/scripts/test{test_case}_client{client_id.upper()}.py'
    ssh_cmd = f'source ~/739p1.env && python {script_name}'
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    username = getpass.getuser()
    home_dir = os.path.expanduser('~')
    print(username)
    print(home_dir)
    assert username is not None
    sshkey_fname = f'{home_dir}/.ssh/id_rsa'
    print(f'Connect {username}@{host}')
    client.connect(hostname=host, username=username, key_filename=sshkey_fname)
    stdin, stdout, stderr = client.exec_command(ssh_cmd)
    # useful when connection init has issues
    #print(stdout.readlines())
    #print(stderr.readlines())
    #client.close()
    #print(ssh_cmd)


def send_signal(host: str, signal_fname: str):
    """
    Send signal to a remote client so it can keep executing.
    """
    ssh_cmd = f'ssh {host} touch {signal_fname}'
    outs, _errs, ret = get_shell_cmd_output(None, ssh_cmd)
    for lo in outs:
        print(lo)
    assert ret == 0


def poll_signal_remove(host: str, signal_fname: str) -> bool:
    """
    Poll if a given signal_fname file exists in the remote host.
    """
    rmt_cmd = f'(ls {signal_fname} && echo EXISTS) || echo NOT_EXISTS'
    ssh_cmd = f'ssh {host} "{rmt_cmd}"'
    outs, _errs, ret = get_shell_cmd_output(None, ssh_cmd)
    removed = False
    for lo in outs:
        if 'NOT_EXISTS' in lo:
            removed = True
    if not removed:
        print(f'signal exists: {outs}')
    return removed


def create_file(fname: str):
    fd = os.open(fname, os.O_CREAT)
    if fd < 0:
        raise RuntimeError(f'Cannot create {fname}')
    os.close(fd)


def mkdir(dir_name: str):
    return os.mkdir(dir_name)


def delete_file(fname: str):
    ret = os.unlink(fname)
    assert ret == 0


def stat_file(fname: str) -> os.stat_result:
    file_stat = os.lstat(fname)
    return file_stat


def path_exists(path_name: str) -> bool:
    return os.path.exists(path_name)


def open_file(fname: str) -> int:
    fd = os.open(fname, os.O_RDWR)
    if fd < 0:
        raise RuntimeError(f'open({fname}) return {fd}')
    return fd


def close_file(fd: int):
    os.close(fd)


def write_file(fd: int, w_str: str, start_off: int = -1) -> int:
    assert fd is not None
    w_bytes = bytes(w_str, 'utf-8')
    if start_off >= 0:
        ret = os.pwrite(fd, w_bytes, start_off)
    else:
        ret = os.write(fd, w_bytes)
    return ret


def read_file(fd: int, len: int, start_off: int = -1) -> str:
    if start_off >= 0:
        r_bytes = os.pread(fd, len, start_off)
    else:
        r_bytes = os.read(fd, len)
    ret_str = r_bytes.decode('utf-8')
    return ret_str


def test():
    pass


if __name__ == '__main__':
    test()
