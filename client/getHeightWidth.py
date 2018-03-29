#encoding:utf-8
import os
with open("E:\\Qt\MODELPOINT\\fanshou-left.txt",'r',encoding="utf-8")as f:
	text=f.read()
#with open("E:\\Qt\MODELPOINT\\zhengshou-left.txt",'r',encoding="utf-8")as f:
#	text=f.read()
l1=text.split("\n")
l1=[e[11:] for e in l1]
l2=[e.split(":") for e in l1]
lx=[]
ly=[]
for i in range(len(l2)):
	for j in range(len(l2[i])):
		if j%2==0:
			lx.append(l2[i][j])
		else:
			ly.append(l2[i][j])
lx=[e for e in lx if e != '0'and e!='']
ly=[e for e in ly if e != '0'and e!='']
maxX=max(lx)
minX=min(lx)
maxY=max(ly)
minY=min(ly)
print(l1[0])
print(l2[0])
print(lx)
print(ly)
print(maxX)
print(maxY)
print(minX)
print(minY)
width=float(maxX)-float(minX)+100
height=float(maxY)-float(minY)+100
xpianyi=float(minX)-50
ypianyi=float(minY)-50
print("width   ReplayMDThread 24hang 3*",width)
print("height  ReplayMDThread 24hang 3*",height)
print("xpianyi  readFileThread 4hang 3*",xpianyi)
print("ypianyi   readFileThread 5hang 3*",ypianyi)