#-*- codings: utf-8 -*-
import json, time
from futils import * 

def kill_process(device, package_name):
  result = device.shell('ps -A | grep %s' % package_name)
  lines = result.split('\n')
  for line in lines:
    if len(line.strip()) == 0:
      continue
    columns = line.split()
    if len(columns) < 8:
      continue
    #if columns[-1] != package_name:
    #  continue
    pid = columns[1].strip()
    if len(pid) <= 0 or not str.isnumeric(pid):
      continue
    cmd = 'su -c kill %s' % pid
    print(cmd)
    assert_ret(device.shell(cmd))
    time.sleep(1)
  #print(result)

def start_process(device, package_name):
  assert_ret(device.shell('monkey -p %s -v 500' % package_name))
  
def start_worker(package_name):
  device = get_android_device()
  kill_process(device, package_name)
  time.sleep(15)
  start_process(device, package_name)
  time.sleep(20)

def stop_worker(package_name):
  device = get_android_device()
  kill_process(device, package_name)
  time.sleep(10)

def main():
  package_name = get_package_name()
  start_worker(package_name)
  print('game over!')

if __name__ == '__main__':
  main()
