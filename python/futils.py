#-*- codings: utf-8 -*-
import json
import adbutils

def get_android_device():
  client = adbutils.AdbClient(host="127.0.0.1", port=5037)
  device = client.device(serial='7876c4ab')
  return device

def write_file(filename, mode, data):
  with open(filename, mode=mode) as fo:
    fo.write(data)
    
def assert_ret(ret):
  if ret != '':
    print(ret)


def create_dump_config():
  config = {
    'isEnabled'  : True,
    'isUseGumJs' : True,
    'isDumpDex'  : True,
    'isDumpCode' : True,
    'isDumpCodeByClassList': False,
    'classList'    : [],
    'delaySeconds' : 0,
  }
  return json.dumps(config, indent=2)

def create_frida_config(package_name):
  config = {
    "targets": [
        {
            "app_name" : "%s" % package_name,
            "enabled": True,
            "start_up_delay_ms": 10000,
            "injected_libraries": [
#                {
#                    "path": "/data/data/%s/plugin/libdobby.so" % package_name
#                },
                {
                    "path": "/data/data/%s/plugin/libplugin.so" % package_name
                }
            ]
        }
    ]
  }
  return json.dumps(config, indent=2)

def get_package_name():
  #package_name = 'com.nbbank'                               # 宁波银行       梆梆加固企业版++
  #package_name = 'cn.com.hzb.mobilebank.per'                # 杭州银行       梆梆加固企业版++
  package_name = 'com.tlbank'                               # 泰隆银行       梆梆加固企业版++
  #package_name = 'com.unionpay'                             # 云闪付         梆梆加固企业版
  package_name = 'cn.gov.pbc.dcep'                          # 数字人民币     梆梆加固企业版
  #package_name = 'com.cloudpower.netsale.activity'          # 中国人保       梆梆加固企业版
  #package_name = 'com.sinovatech.unicom.ui'                 # 中国联通       梆梆加固企业版
  #package_name = 'com.cubic.autohome'                       # 汽车之家       梆梆加固企业版
  #package_name = 'com.yitong.mbank.psbc'                    # 邮储银行       梆梆加固企业版
  #package_name = 'com.bankcomm.Bankcomm'                    # 交通银行       梆梆加固企业版
  #package_name = 'com.android.bankabc'                      # 中国农业银行   梆梆加固企业版
  #package_name = 'com.chinamworld.main'                     # 中国建设银行   梆梆加固企业版
  #package_name = 'com.icbc'                                 # 中国工商银行   梆梆加固企业版
  #package_name = 'com.chinamworld.bocmbci'                  # 中国银行       梆梆加固企业版
  #package_name = 'com.cib.cibmb'                            # 兴业银行       爱加密企业版--
  #package_name = 'com.cib.xyk'                              # 兴业生活       爱加密企业版--
  #package_name = 'com.cib.xykb'                             # 兴业生活商户版 爱加密企业版
  #package_name = 'com.ct.client'                            # 中国电信       爱加密企业版+?
  #package_name = 'com.chinatelecom.bestpayclient'           # 翼支付         爱加密企业版--
  #package_name = 'com.greenpoint.android.mc10086.activity'  # 中国移动       爱加密企业版++
  #package_name = 'com.webank.wemoney'                       # 微众银行       腾讯御安全++
  #package_name = 'com.coolapk.market'                       # 酷安           网易易盾++
  #package_name = 'com.tmri.app.main'                        # 安管12123      网易易盾--
  #package_name = 'com.sgcc.wsgw.cn'                         # 网上国网       娜迦加固++
  #package_name = 'com.xiyou.english'                        # XIYOU英语      360加固++
  #package_name = 'abc.cookie.udex'
  #package_name = 'abc.cookie.plugin'
  return package_name
