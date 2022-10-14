import sys
import numpy as np
import pandas as pd
import ROOT as r
import fedrarootlogon

def builddataframe(brick, path = "..", cutstring = "1", major = 0, minor = 0, newzprojection = None):
 """build pandas dataframe starting from couples and scanset 
    brick = Number of brick as in b0000*
    path = input path to the folder containing theb b0000* folder
    cutsring = eventual selection to couples
    newzprojection = list of projection to a new z reference system
 """
 nplate =0
 print("Reading ScanSet at path ",path)

 #reading scanset
 sproc = r.EdbScanProc()
 sproc.eProcDirClient=path
 id = r.EdbID(brick,nplate,major,minor)
 ss = sproc.ReadScanSet(id)
 ss.Brick().SetID(brick)
 
 #preparing patterns
 npl = ss.eIDS.GetEntries()

 cut = r.TCut(cutstring)

 #intial empty arrays
 IDall = np.zeros(0,dtype=int)
 PIDall = np.zeros(0,dtype=int)

 xall = np.zeros(0,dtype=np.float32)
 yall = np.zeros(0,dtype=np.float32)
 zall = np.zeros(0,dtype=np.float32)
 TXall = np.zeros(0,dtype=np.float32)
 TYall = np.zeros(0,dtype=np.float32)

 CHI2all = np.zeros(0,dtype=np.float32)
 Weightall = np.zeros(0,dtype=np.float32)
 Volumeall = np.zeros(0,dtype=np.float32)

 print ("Cut on couples ")
 cut.Print()

 print("Try to open folders at path ",path+"/b0000"+str(brick))
 for i in range(npl):
  idplate = ss.GetID(i)
      
  nplate = idplate.ePlate
  plate = ss.GetPlate(idplate.ePlate)
  #read pattern information
  p = r.EdbPattern()

  ect = r.EdbCouplesTree()
  if (nplate) <10:
   ect.InitCouplesTree("couples",path+"/b{:06d}/p00{}/{}.{}.{}.{}.cp.root".format(brick,nplate,brick,nplate,major,minor),"READ")
  elif (nplate) < 100:
   ect.InitCouplesTree("couples",path+"/b{:06d}/p0{}/{}.{}.{}.{}.cp.root".format(brick,nplate,brick,nplate,major,minor),"READ")
  else:
   ect.InitCouplesTree("couples",path+"/b{:06d}/p{}/{}.{}.{}.{}.cp.root".format(brick,nplate,brick,nplate,major,minor),"READ") 

  #addingcut
  ect.eCut = cut 
  cutlist = ect.InitCutList()
  
  nsegcut = cutlist.GetN()
  nseg = ect.eTree.GetEntries()

  IDarray_plate = np.zeros(nsegcut,dtype=int)
  PIDarray_plate = np.zeros(nsegcut,dtype=int)

  xarray_plate = np.zeros(nsegcut,dtype=np.float32)
  yarray_plate = np.zeros(nsegcut,dtype=np.float32)
  zarray_plate = np.zeros(nsegcut,dtype=np.float32)
  TXarray_plate = np.zeros(nsegcut,dtype=np.float32)
  TYarray_plate = np.zeros(nsegcut,dtype=np.float32)

  CHI2array_plate = np.zeros(nsegcut,dtype=np.float32)
  Weightarray_plate = np.zeros(nsegcut,dtype=np.float32)
  Volumearray_plate = np.zeros(nsegcut,dtype=np.float32)
 
  print ("loop on {} segments over  {} for plate {}".format(nsegcut, nseg,nplate))
  for ientry in range(nsegcut):
   iseg = cutlist.GetEntry(ientry)
   ect.GetEntry(iseg)
 
   seg=ect.eS
   #//setting z and affine transformation
   seg.SetZ(plate.Z())
   seg.SetPID(i)
   seg.Transform(plate.GetAffineXY())

   if(newzprojection is not None):
    seg.PropagateTo(newzprojection[i])

   IDarray_plate[ientry] = seg.ID()
   PIDarray_plate[ientry] = seg.PID()
   
   xarray_plate[ientry] = seg.X()
   yarray_plate[ientry] = seg.Y()
   zarray_plate[ientry] = seg.Z()
   TXarray_plate[ientry] = seg.TX()
   TYarray_plate[ientry] = seg.TY()

   CHI2array_plate[ientry] = seg.Chi2()
   Weightarray_plate[ientry] = seg.W()
   Volumearray_plate[ientry] = seg.Volume()

  #end of loop, storing them in global arrays
  IDall = np.concatenate((IDall,IDarray_plate),axis=0)
  PIDall = np.concatenate((PIDall,PIDarray_plate),axis=0)

  xall = np.concatenate((xall,xarray_plate),axis=0)
  yall = np.concatenate((yall,yarray_plate),axis=0)
  zall = np.concatenate((zall,zarray_plate),axis=0)
  TXall = np.concatenate((TXall,TXarray_plate),axis=0)
  TYall = np.concatenate((TYall,TYarray_plate),axis=0)

  CHI2all = np.concatenate((CHI2all,CHI2array_plate),axis=0)
  Weightall = np.concatenate((Weightall,Weightarray_plate),axis=0)
  Volumeall = np.concatenate((Volumeall,Volumearray_plate),axis=0)


 data = {'ID':IDall,'PID':PIDall,'x':xall,'y':yall,'z':zall,'TX':TXall,'TY':TYall, 'Volume':Volumeall, 'W':Weightall, 'CHI2':CHI2all}
 df = pd.DataFrame(data, columns = ['ID','PID','x','y','z','TX','TY','Volume','W','CHI2'])

 return df

def applyconversion(nbrick):
 '''convert couples ROOT files into a csv'''

 #df = builddataframe(nbrick,  cutstring = "eCHI2P<2.0&&s.eW>10&&eN1==1&&eN2==1")
 df = builddataframe(nbrick,  cutstring = "eN1==1&&eN2==1")

 return df

#the two steps can now be done together, without an intermediate file
nbrick = int(sys.argv[1])
#starting all conversion steps
df = applyconversion(nbrick)

df['Plate'] = 7 - df['PID']
df = df.drop(labels='PID',axis=1)

df.to_csv('brick{}_twoplates.csv'.format(nbrick),index=False)


