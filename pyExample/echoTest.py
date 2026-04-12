import echoBase
import time 


eb = echoBase.EchoBase(debug=True)
eb.init(sample_rate=8000)
eb.setSpeakerVolume(90)
eb.play("/media/test8000mono.wav")
#eb.record("/media/mist.bin",100000)

