#-*- codings: utf-8 -*-
from futils import *
from create_work_env import prepare_workenv, clear_workenv
from start_worker import start_worker, stop_worker
from get_dump_result import get_work_result

def dump_dex(package_name):
  print('开始清理工作环境')
  clear_workenv(package_name)  
  frida_config = create_frida_config(package_name)
  dump_config = create_dump_config()
  print('开始准备工作环境: %s' % package_name)
  prepare_workenv(package_name, frida_config, dump_config)
  print('开始启动程序')
  start_worker(package_name)
  ch = None
  while ch != 'Y' and ch != 'y':
    print('开始获取结果')
    get_work_result(package_name)
    ch = input('是否成功获取结果，如果是，则输入 Y ，否则输入 N: ')
  print('开始结束程序')
  stop_worker(package_name)
  print('开始清理工作环境')
  clear_workenv(package_name)
  print('工作完成!')
  
def main():
  package_name = get_package_name()
  dump_dex(package_name)
  print('game over!')

if __name__ == '__main__':
  main()


