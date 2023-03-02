#!/usr/bin/python3
import time
import struct
from pygltflib import GLTF2
import time
import random
import numpy

# Grafik
from threading import Thread, Event
import tkinter
from PIL import Image, ImageTk, ImageDraw
# Lyd
from pydub import AudioSegment
from pydub.playback import play

# Opret vindue
window = tkinter.Tk(className="Christians Spil")
window.attributes('-fullscreen',True)

# Opret laerred
#bredde=1326
#hoejde=850
bredde=320
hoejde=240
visning=Image.new('RGB',(bredde,hoejde))
visning.putpixel((0,0),(1,2,3))
canvas=numpy.array(visning)
#print(str(canvas))

zbuf=[20.0]*(hoejde*bredde)
# Opsaet visning til vindue
tk_visning = ImageTk.PhotoImage(visning)
windowContent=tkinter.Label(window,image=tk_visning)
windowContent.pack()

def laes_gltf(filnavn,x,y,z,r,g,b):
  gltf = GLTF2().load(filnavn)
  res=[]
  for primitive in gltf.meshes[0].primitives:
    # get the binary data for this mesh primitive from the buffer
    accessor = gltf.accessors[primitive.attributes.POSITION]
    bufferView = gltf.bufferViews[accessor.bufferView]
    buffer = gltf.buffers[bufferView.buffer]
    data = gltf.get_data_from_buffer_uri(buffer.uri)

    # pull each vertex from the binary buffer and convert it into a tuple of python floats
    vertices = []
    for i in range(accessor.count):
        index = bufferView.byteOffset + accessor.byteOffset + i*12  # the location in the buffer of this vertex
        d = data[index:index+12]  # the vertex data
        v = struct.unpack("<fff", d)   # convert from base64 to three floats
        print(i, v)
        vertices.append(v)
    while len(vertices)>=3:
      res.append({'x1': vertices[0][0]+x,
                  'y1': vertices[0][1]+y,
                  'z1': vertices[0][2]+z,
                  'x2': vertices[1][0]+x,
                  'y2': vertices[1][1]+y,
                  'z2': vertices[1][2]+z,
                  'x3': vertices[2][0]+x,
                  'y3': vertices[2][1]+y,
                  'z3': vertices[2][2]+z,
                  'roed': r,
                  'groen': g,
                  'blaa': b
                 })
      vertices=vertices[3:]
  return res

def laes_real(bs):
  return struct.unpack('<f',bs)[0]
  
# Indlæs 3d stl fil til trekants liste
def laes_stl(fnavn,x,y,z,r,g,b):
  res=[]
  f=open(fnavn,'rb')
  header=f.read(80)
  count=int.from_bytes(f.read(4),byteorder='little')
  #print('laes_stl #trekanter='+str(count))
  for t in range(count):
    x0b=f.read(4)
    x0=struct.unpack('<f',x0b)[0]
    y0b=f.read(4)
    y0=struct.unpack('<f',y0b)[0]
    z0b=f.read(4)
    z0=struct.unpack('<f',z0b)[0]
    x1b=f.read(4)
    x1=laes_real(x1b)
    y1b=f.read(4)
    y1=laes_real(y1b)
    z1b=f.read(4)
    z1=laes_real(z1b)
    x2b=f.read(4)
    x2=laes_real(x2b)
    y2b=f.read(4)
    y2=laes_real(y2b)
    z2b=f.read(4)
    z2=laes_real(z2b)
    x3b=f.read(4)
    x3=laes_real(x3b)
    y3b=f.read(4)
    y3=laes_real(y3b)
    z3b=f.read(4)
    z3=laes_real(z3b)
    att=f.read(2)
    #print('x1 bytes: '+str(x1b))
    #print('point1: '+str((x1,y1,z1)))
    #print('point2: '+str((x2,y2,z2)))
    #print('point3: '+str((x3,y3,z3)))
    res.append({'x1': x1+x,
                'y1': y1+y,
                'z1': z1+z,
                'x2': x2+x,
                'y2': y2+y,
                'z2': z2+z,
                'x3': x3+x,
                'y3': y3+y,
                'z3': z3+z,
                'roed':  random.randint(0,255), #r,
                'groen': random.randint(0,255), #g,
                'blaa':  random.randint(0,255) #b
               })
  f.close()
  return res
  
#print(str(laes_gltf('kube.glb',0,50,0,10,10,200)))
  
# Lav Start-tilstand
tilstand = {'venstre': False,
            'hoejre': False,
            'op': False,
            'ned': False,
            'mellemrum': False,
            'trekanter': laes_stl('froring.stl',0,100,0,10,10,200),#+laes_stl('chr.stl',50,100,-10,30,250,10),
            'mig_x': 0.0,
            'mig_y': 0.0,
            'mig_z': 0.0,
            'retning_x': 0.0,
            'retning_y': 1.0,
            'retning_z': 0.0,
            'quit': False
           }
# Haandter tastetryk, musetryk mm.
def modtag_tryk(event):
  global tilstand
  #print("Event: "+str(event))
  if (str(event.type)=='KeyPress' or str(event.type)=='2') and event.keysym=='Left':
    tilstand['venstre']=True
  elif (str(event.type)=='KeyRelease' or str(event.type)=='3') and event.keysym=='Left':
    tilstand['venstre']=False
  elif (str(event.type)=='KeyPress' or str(event.type)=='2') and event.keysym=='Right':
    tilstand['hoejre']=True
  elif (str(event.type)=='KeyRelease' or str(event.type)=='3') and event.keysym=='Right':
    tilstand['hoejre']=False
  elif (str(event.type)=='KeyPress' or str(event.type)=='2') and event.keysym=='Up':
    tilstand['op']=True
  elif (str(event.type)=='KeyRelease' or str(event.type)=='3') and event.keysym=='Up':
    tilstand['op']=False
  elif (str(event.type)=='KeyPress' or str(event.type)=='2') and event.keysym=='Down':
    tilstand['ned']=True
  elif (str(event.type)=='KeyRelease' or str(event.type)=='3') and event.keysym=='Down':
    tilstand['ned']=False
  elif (str(event.type)=='KeyPress' or str(event.type)=='2') and event.keysym=='space':
    tilstand['mellemrum']=True
  elif (str(event.type)=='KeyRelease' or str(event.type)=='3') and event.keysym=='space':
    tilstand['mellemrum']=False
  elif (str(event.type)=='KeyPress' or str(event.type)=='2') and event.keysym=='Escape':
    tilstand['quit']=True
  #else:
  #  print('Ukendt tilstand: '+str(event.type)+' '+str(event.keysym))

#window.force_focus()
window.bind("<Button>", modtag_tryk)
window.bind("<KeyPress>", modtag_tryk)
window.bind("<KeyRelease>", modtag_tryk)

def tegn_hline(y,x1,d1,x2,d2,r,g,b):
  #print('tegn_hline '+str((y,x1,d1,x2,d2,r,g,b)))
  global canvas
  global zbuf
  global bredde
  global hoejde

  if x2<x1:
    return tegn_hline(y,x2,d2,x1,d1,r,g,b)

  if y<0.0:
    return
  if y>=hoejde:
    return
  if x2<0.0:
    return
  if x1>=bredde:
    return

  if x1<0:
    d1=d1-(d2-d1)*x1/(x2-x1)
    x1=0
  if x2>bredde:
    d2=d2+(d1-d2)*(x2-bredde)/(x2-x1)
    x2=bredde
  if d1<=5.0 and d2<=5.0:
    return
  if d1>=200.0 and d2>=200.0:
    return
    
  if int(x2)>int(x1):
    trind=(d2-d1)/(x2-x1)
    posd=d1
    for x in range(int(x1),int(x2)):
      if posd>5.0 and posd<zbuf[int(x)+int(y)*bredde]:
        canvas[int(y),int(x)][0]=r
        canvas[int(y),int(x)][1]=g
        canvas[int(y),int(x)][2]=b
        #visning.putpixel((int(x),int(y)),(r,g,b))
        zbuf[int(x)+int(y)*bredde]=posd
      posd+=trind
    
def tegn_trekant(x1,y1,d1,x2,y2,d2,x3,y3,d3,r,g,b):
  global visning
  global bredde
  global hoejde

  # Test om vi kan afvise at tegne
  if d1<=5.0 or d2<=5.0 or d3<=5.0:
    return

  if x1<0.0 and x2<0.0 and x3<0.0:
    return

  if x1>=bredde and x2>=bredde and x3>=bredde:
    return

  if y1<0.0 and y2<0.0 and y3<0.0:
    return

  if y1>=hoejde and y2>=hoejde and y3>=hoejde:
    return

  # Sorter hjørner
  if y2<y1:
    return tegn_trekant(x2,y2,d2,x1,y1,d1,x3,y3,d3,r,g,b)

  if y3<y1:
    return tegn_trekant(x3,y3,d3,x2,y2,d2,x1,y1,d1,r,g,b)

  if y3<y2:
    return tegn_trekant(x1,y1,d1,x3,y3,d3,x2,y2,d2,r,g,b)

  # Tegn hjørner
  if x1>0.0 and x1<bredde and y1>=0.0 and y1<hoejde:
    visning.putpixel((int(x1),int(y1)),(200-int(d1*10),200-int(d1*10),255))
  if x2>0.0 and x2<bredde and y2>=0.0 and y2<hoejde:
    visning.putpixel((int(x2),int(y2)),(255,200-int(d2*10),200-int(d1*10)))
  if x3>0.0 and x3<bredde and y3>=0.0 and y3<hoejde:
   visning.putpixel((int(x3),int(y3)),(200-int(d2*10),255,200-int(d1*10)))


  # Tegn hver hlinje
  posx2=x1
  posx3=x1
  posd2=d1
  posd3=d1
  posy=y1
  if int(y2)>int(y1): # Tegn ned til y2
    trinx2=(x2-x1)/(y2-y1)
    trind2=(d2-d1)/(y2-y1)
    trinx3=(x3-x1)/(y3-y1)
    trind3=(d3-d1)/(y3-y1)
    for y in range(int(y2)-int(y1)):
      tegn_hline(posy,posx2,posd2,posx3,posd3,r,g,b)
      posx2+=trinx2
      posd2+=trind2
      posx3+=trinx3
      posd3+=trind3
      posy+=1
  posx2=x2
  posd2=d2
  if int(y3)>int(y2): # Tegn ned til y3
    trinx2=(x3-x2)/(y3-y2)
    trind2=(d3-d2)/(y3-y2)
    trinx3=(x3-x1)/(y3-y1)
    trind3=(d3-d1)/(y3-y1)
    for y in range(int(y3)-int(y2)):
      tegn_hline(posy,posx2,posd2,posx3,posd3,r,g,b)
      posx2+=trinx2
      posd2+=trind2
      posx3+=trinx3
      posd3+=trind3
      posy+=1

# Lav spillet
def gameloop(tilstand):
  global window
  global windowContent
  global tk_visning
  global visning
  global canvas
  global zbuf
  global hoejde
  global bredde
  # Indlaes billeder
  #imgBack = new = Image.new(mode="RGBA", size=(bredde,hoejde))
  #imgBack = Image.open('baggrund.png')
  #imgBold = Image.open('fodbold.png')
  #imgBold=imgBold.convert('RGBA')
  #imgBold=imgBold.resize((50,50),Image.LANCZOS)
  #imgMan = Image.open('man.png')
  #imgMan=imgMan.convert('RGBA')
  #imgMan=imgMan.resize((75,100),Image.LANCZOS)
  #imgAligator = Image.open('aligator.png')
  #imgAligator=imgAligator.convert('RGBA')
  #imgAligator=imgAligator.resize((75,100),Image.LANCZOS)
  #imgStart = Image.open('start.png')
  #imgStart=imgStart.convert('RGBA')
  #draw=ImageDraw.Draw(visning)

  #sndJump=AudioSegment.from_mp3("jump.mp3")
  # Mal sort
  #for x in range(bredde):
  #  for y in range(hoejde):
  #    visning.putpixel((x,y),(0,0,0))

  t=time.time()
  while True: # Goer dette imens spillet koerer
    if tilstand['quit']:
      window.quit()
      break
    # Bevaeg spilleren
    if tilstand['op']:
      tilstand['mig_x']=tilstand['mig_x']+tilstand['retning_x']
      tilstand['mig_y']=tilstand['mig_y']+tilstand['retning_y']
      tilstand['mig_z']=tilstand['mig_z']+tilstand['retning_z']
    if tilstand['ned']:
      tilstand['mig_x']=tilstand['mig_x']-tilstand['retning_x']
      tilstand['mig_y']=tilstand['mig_y']-tilstand['retning_y']
      tilstand['mig_z']=tilstand['mig_z']-tilstand['retning_z']
    if tilstand['venstre']:
      # antag retning_z=0
      tilstand['mig_x']=tilstand['mig_x']-tilstand['retning_y']
      tilstand['mig_y']=tilstand['mig_y']+tilstand['retning_x']
    if tilstand['hoejre']:
      # antag retning_z=0
      tilstand['mig_x']=tilstand['mig_x']+tilstand['retning_y']
      tilstand['mig_y']=tilstand['mig_y']-tilstand['retning_x']
    if tilstand['mellemrum']: # Skyd
      tilstand['mig_z']=tilstand['mig_z']+1.0
      
    if tilstand['mig_z']>0.0:
      tilstand['mig_z']-=0.1

    # Tegn 3D
    # Mal sort
    #draw.rectangle((0,0,bredde,hoejde), fill=(0,0,0,0))
    canvas.fill(0)
    zbuf=[200.0]*bredde*hoejde

    # Mal trekanter
    for trekant in tilstand['trekanter']:
      #print('trekant: '+str(trekant))
      tx1=trekant['x1']-tilstand['mig_x']
      ty1=trekant['y1']-tilstand['mig_y']
      tz1=trekant['z1']-tilstand['mig_z']
      tx2=trekant['x2']-tilstand['mig_x']
      ty2=trekant['y2']-tilstand['mig_y']
      tz2=trekant['z2']-tilstand['mig_z']
      tx3=trekant['x3']-tilstand['mig_x']
      ty3=trekant['y3']-tilstand['mig_y']
      tz3=trekant['z3']-tilstand['mig_z']
      
      if ty1==0.0:
        ty1=0.000001
      if ty2==0.0:
        ty2=0.000001
      if ty3==0.0:
        ty3=0.000001
        
      # Antag retning x=z=0 og y=1
      x1=(0.5+tx1/ty1)*bredde
      z1=(0.5-tz1/ty1)*hoejde
      x2=(0.5+tx2/ty2)*bredde
      z2=(0.5-tz2/ty2)*hoejde
      x3=(0.5+tx3/ty3)*bredde
      z3=(0.5-tz3/ty3)*hoejde
      
      #draw.polygon([(x1,z1),(x2,z2),(x3,z3)],fill=(trekant['roed'],trekant['groen'],trekant['blaa'],255))
      tegn_trekant(x1,z1,ty1,x2,z2,ty2,x3,z3,ty3,trekant['roed'],trekant['groen'],trekant['blaa'])
    
    
    visning=Image.fromarray(canvas)
    draw=ImageDraw.Draw(visning)
    draw.rectangle((0,0,100,30), fill=(100,100,100,0))
    t2=time.time()
    fps=1.0/(t2-t)
    t=t2
    draw.text((10,10),'FPS: %.2f' % fps	)
    tk_visning = ImageTk.PhotoImage(visning)
    windowContent.configure(image=tk_visning)
    windowContent.image=tk_visning
    #print('frame')
    time.sleep(0.01)

# Sig at spillet skal starte
thread_gameloop = Thread(target=gameloop, args=(tilstand,))
thread_gameloop.start()

# Koer vinduet
tkinter.mainloop()
