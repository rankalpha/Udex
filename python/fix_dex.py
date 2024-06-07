#-*- codings: utf-8 -*-
import os, base64, shutil
from futils import *

def lookup_files(dir_name, ext_name):
  filenames = []
  for fname in os.listdir(dir_name):
    if os.path.splitext(fname)[-1] != ext_name:
      continue
    filename = os.path.abspath(os.path.join(dir_name, fname))
    filenames.append(filename)
  return filenames

def read_file(filename, mode):
  with open(filename, mode=mode) as fi:
    return fi.read()

CONVERT_FUNCS = {
  0: str,
  1: int,
  2: int,
  3: int,
  4: base64.decodebytes,
}

def parse_code_item(line):
  assert(line[0] == '{' and line[-1] == '}')
  cols = line[1:-1].split(',')
  assert(len(cols) == 5)
  values = []
  for index, col in enumerate(cols):
    kv_items = col.split(':')
    assert(len(kv_items) == 2)
    str_value = kv_items[1]
    convert_fun = CONVERT_FUNCS[index]
    real_value = convert_fun(str_value.encode('latin1'))
    values.append(real_value)
  return values
    
def parse_idx_file(idx_file):
  data = read_file(idx_file, 'r')
  lines = data.split('\n')
  code_items = []
  for line in lines:
    if len(line.strip()) == 0:
      continue
    code_item = parse_code_item(line)
    code_items.append(code_item)
  return code_items

def fix_dex(package_name, is_local_fix):
  output_path = 'output/%s' % package_name
  dex_files = lookup_files(output_path, '.dex')
  for dex_file in dex_files:
    base_name = os.path.splitext(dex_file)[0]
    idx_file = base_name + '.idx'
    if not os.path.exists(idx_file):
      continue
    if is_local_fix:
      new_dex_file = dex_file
    else:
      new_dex_file = base_name + '.new.dex'
      shutil.copyfile(dex_file, new_dex_file)
    code_items = parse_idx_file(idx_file)
    print('fix %s file: %s methods' % (dex_file, len(code_items)))
    with open(new_dex_file, 'r+b') as fo:
      for name, method_idx, offset, code_item_len, code_item in code_items:
        assert(len(code_item) == code_item_len)
        fo.seek(offset, os.SEEK_SET)
        fo.write(code_item)
        fo.flush()
        
def main():
  is_local_fix = True
  package_name = get_package_name()
  fix_dex(package_name, is_local_fix)
  print('game over!')

if __name__ == '__main__':
  main()
