#! /usr/bin/python
# -*- coding: utf-8 -*-

import fractions
import gmpy

#----------------------------------
# Key
#----------------------------------

#------------ DSA params ------------
pub = 0x933F063AAA6DE378FBE0723BCCA94001B69E7B2C0490D2980847ECD820DE6983C530F48A52C358E901DEF14876487904BC0F63F68D03AFB8A9B09CF7734CBB8E
priv = 0x664AA5A55F4A3A2CA57504B2D891E010CB5A70B5
sha2 = 0x1D76417E4EA8B96640F24369F0B9F60FE8EF8490
G = 0x3E612EF530F0F82D1FC87E651E61C84DAFC50529B8C947205E2FA1B8DCB593D1B17169AC6A87E5B10A44F0E5D95DE4569E60F6778784156851170592DCD003B1
P = 0xA486FEC4C33904CD9508B6B44D45791172525ECD018827FD447B25E4F51BDFA048E330AFE1007009365C271507DD06DA28EBB4D133CB225737C9E6BB061E3DE7
Q = 0xCF9319C41973314095F4AE4A8251D7F3781EAA03
k = 0x87DB5C1E3954C2CFE395BF24E957187F20B0B088
#------------------------------------
#------------------------------------


#----------------------------------
#----------------------------------
def dsa_sign():
  k_inv = gmpy.invert(k,Q)
  print "k_inv: 0x%X" % k_inv
  r = pow(G,k,P)
  r = r % Q
  print "r: 0x%X" % r
  s = (r*priv) % Q
  s = (s + sha2) % Q
  s = (s * k_inv) % Q
  print "s: 0x%X" % s
  return [r,s]
  
def dsa_check(r,s):
  w = gmpy.invert(s,Q)
  print "w: 0x%X" % w
  u1 = (sha2 * w) % Q
  print "u1: 0x%X" % u1
  u2 = (r * w) % Q
  print "u2: 0x%X" % u2
  v = (pow(G,u1,P) * pow(pub,u2,P)) % P % Q
  print "v: 0x%X" % v


#----------------------------------
#----------------------------------
rs = dsa_sign()
dsa_check(rs[0],rs[1])




