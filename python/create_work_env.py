#-*- codings: utf-8 -*-
import os
from futils import * 

def get_abs_path(path):
  return os.path.abspath(path)

def push_frida_config(device, frida_config):
  write_file('output/config.json', 'w', frida_config)
  assert_ret(device.sync.push('output/config.json', '/data/local/tmp/frisk/config.json'))

def clear_frida_config(device):
  assert_ret(device.shell('su -c rm /data/local/tmp/frisk/config.json'))

def push_dump_config(device, package_name, dump_config):
  write_file('output/dump.json', 'w', dump_config)
  assert_ret(device.shell('su -c mkdir /data/local/tmp/plugin'))
  assert_ret(device.shell('su -c chmod a+w /data/local/tmp/plugin'))
  assert_ret(device.sync.push('output/dump.json', '/data/local/tmp/plugin/dump.json'))
  assert_ret(device.shell('su -c mkdir /data/data/%s/plugin' % package_name))
  assert_ret(device.shell('su -c chmod a+w /data/data/%s/plugin' % package_name))  
  assert_ret(device.shell('su -c cp /data/local/tmp/plugin/dump.json /data/data/%s/plugin/dump.json' % package_name))

def copy_gumjs_file(device, package_name):
  assert_ret(device.shell('su -c mkdir /data/local/tmp/plugin/gumjs'))
  assert_ret(device.shell('su -c mkdir /data/data/%s/plugin/gumjs' % package_name))
  assert_ret(device.shell('su -c chmod a+w /data/local/tmp/plugin/gumjs'))
  assert_ret(device.shell('su -c chmod a+w /data/data/%s/plugin/gumjs' % package_name))
  assert_ret(device.sync.push('output/main.js', '/data/local/tmp/plugin/gumjs/main.js'))
  assert_ret(device.shell('su -c cp /data/local/tmp/plugin/gumjs/main.js /data/data/%s/plugin/gumjs/main.js' % package_name))

def push_dobby_loader(device, package_name):
  #assert_ret(device.sync.push('../app/build/intermediates/merged_native_libs/debug/mergeDebugNativeLibs/out/lib/arm64-v8a/libdobby.so',  '/data/local/tmp/plugin/libdobby.so'))
  assert_ret(device.sync.push('../app/build/intermediates/merged_native_libs/debug/mergeDebugNativeLibs/out/lib/arm64-v8a/libplugin.so', '/data/local/tmp/plugin/libplugin.so'))
  #assert_ret(device.shell('su -c cp /data/local/tmp/plugin/libdobby.so  /data/data/%s/plugin/libdobby.so' % package_name))
  assert_ret(device.shell('su -c cp /data/local/tmp/plugin/libplugin.so /data/data/%s/plugin/libplugin.so' % package_name))

def push_dump_injection(device, package_name):
  assert_ret(device.shell('su -c mkdir /data/local/tmp/plugin/udex'))
  assert_ret(device.shell('su -c mkdir /data/data/%s/plugin' % package_name))
  assert_ret(device.shell('su -c mkdir /data/data/%s/plugin/udex' % package_name))
  assert_ret(device.shell('su -c chmod a+w /data/local/tmp/plugin/udex'))
  assert_ret(device.shell('su -c chmod a+w /data/data/%s/plugin' % package_name))
  assert_ret(device.shell('su -c chmod a+w /data/data/%s/plugin/udex' % package_name))
  assert_ret(device.sync.push('../udex/build/intermediates/merged_native_libs/debug/mergeDebugNativeLibs/out/lib/arm64-v8a/libudex.so', '/data/local/tmp/plugin/udex/libudex.so'))
  assert_ret(device.sync.push('../udex/build/intermediates/dex/debug/mergeDexDebug/classes.dex', '/data/local/tmp/plugin/udex/classes.dex'))
  assert_ret(device.shell('su -c cp /data/local/tmp/plugin/udex/libudex.so  /data/data/%s/plugin/udex/libudex.so' % package_name))
  assert_ret(device.shell('su -c cp /data/local/tmp/plugin/udex/classes.dex /data/data/%s/plugin/udex/classes.dex' % package_name))
  assert_ret(device.shell('su -c chmod a-w /data/data/%s/plugin/udex/classes.dex' % package_name))

def create_dump_directory(device, package_name):
  assert_ret(device.shell('su -c rm /data/data/%s/dump/*.*' % package_name))
  assert_ret(device.shell('su -c mkdir /data/data/%s/dump' % package_name))
  assert_ret(device.shell('su -c chmod a+r /data/data/%s/dump' % package_name))
  assert_ret(device.shell('su -c chmod a+w /data/data/%s/dump' % package_name))

def prepare_workenv(package_name, frida_config, dump_config):
  device = get_android_device()
  if package_name in ['abc.cookie.plugin', 'abc.cookie.udex']:
    clear_frida_config(device)
  else:
    push_frida_config(device, frida_config)
  push_dump_config(device, package_name, dump_config)
  copy_gumjs_file(device, package_name)
  push_dobby_loader(device, package_name)
  push_dump_injection(device, package_name)
  create_dump_directory(device, package_name)

def remove_dobby_loader(device, package_name):
  #assert_ret(device.shell('su -c rm /data/data/%s/plugin/libdobby.so' % package_name))
  assert_ret(device.shell('su -c rm /data/data/%s/plugin/libplugin.so' % package_name))

def remove_dump_injection(device, package_name):
  assert_ret(device.shell('su -c rm -rf /data/data/%s/plugin/udex' % package_name))
  assert_ret(device.shell('su -c rm -rf /data/data/%s/plugin' % package_name))

def remove_dump_directory(device, package_name):
  assert_ret(device.shell('su -c rm /data/data/%s/dump/*.*' % package_name))
  assert_ret(device.shell('su -c rm -rf /data/data/%s/dump' % package_name))
  assert_ret(device.shell('su -c rm -rf /data/local/tmp/dump'))

def remove_tmp_files(device):
  assert_ret(device.shell('su -c rm /data/local/tmp/plugin/dump.json'))
  #assert_ret(device.shell('su -c rm /data/local/tmp/plugin/libdobby.so'))
  assert_ret(device.shell('su -c rm /data/local/tmp/plugin/libplugin.so'))
  assert_ret(device.shell('su -c rm /data/local/tmp/plugin/udex/libudex.so'))
  assert_ret(device.shell('su -c rm /data/local/tmp/plugin/udex/classes.dex'))
  
def clear_workenv(package_name):
  device = get_android_device()
  remove_dobby_loader(device, package_name)
  remove_dump_injection(device, package_name)
  remove_dump_directory(device, package_name)
  remove_tmp_files(device)

def main():
  package_name = get_package_name()
  frida_config = create_frida_config(package_name)
  dump_config = create_dump_config()
  clear_workenv(package_name)
  prepare_workenv(package_name, frida_config, dump_config)
  print('game over!')

if __name__ == '__main__':
  main()
