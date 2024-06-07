#-*- codings: utf-8 -*-
import os, shutil
from futils import * 

def get_work_result(package_name):
  device = get_android_device()
  output_path = 'output/%s' % package_name
  if os.path.exists(output_path) and os.path.isdir(output_path):
    shutil.rmtree(output_path)
  assert_ret(device.shell('su -c mkdir /data/local/tmp/dump'))    
  assert_ret(device.shell('su -c rm /data/local/tmp/dump/*.*'))
  assert_ret(device.shell('su -c chmod a+r /data/data/%s/dump/*.*' % package_name))
  assert_ret(device.shell('su -c cp /data/data/%s/dump/*.* /data/local/tmp/dump/' % package_name))
  assert_ret(device.sync.pull('/data/local/tmp/dump/', output_path))

def main():
  package_name = get_package_name()
  get_work_result(package_name)
  print('game over!')

if __name__ == '__main__':
  main()
