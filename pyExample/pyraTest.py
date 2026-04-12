import echoPyr as echoBase
import time 
from i2c_bus import I2CBus
from stm32_ctrl import STM32Ctrl
from aw87559 import AW87559
from aw87559 import AW87559_ADP_PASS_THROUGH, AW87559_ADP_FORCE_BOOST
from aw87559 import AW87559_GAIN_0DB, AW87559_GAIN_6DB, AW87559_GAIN_12DB,AW87559_GAIN_18DB,AW87559_GAIN_24DB
from si5351 import SI5351



i2cbus = I2CBus()
i2cbus.begin(id=0, sda=38, scl=39)

print(i2cbus._i2c.scan())

stm32 = STM32Ctrl(i2cbus)
stm32.begin()
stm32.set_rgb(2,0,100,0,0)
stm32.set_rgb(2,1,0,100,0)
stm32.set_rgb(2,2,0,0,100)
stm32.set_rgb(2,10,100,0,0)
stm32.set_rgb(2,11,0,100,0)
stm32.set_rgb(2,12,0,0,100)
stm32.reset_speaker()
time.sleep(.1)


#clk = SI5351(bus=i2cbus)
#clk.begin()
#clk.set_sample_rate(2048000) 

eb = echoBase.EchoBase(debug=True)
eb.init(i2c=i2cbus._i2c, sample_rate=8000)
print("EB ready")
time.sleep(.2)

pa = AW87559(i2cbus)
pa.begin()
pa.set_pa_gain(AW87559_GAIN_6DB)
r = pa.get_adp_mode()
print("ADP mode:", r)
r = pa.get_pa_gain()
print("PA gain:", r)
time.sleep(.1)


eb.setSpeakerVolume(60)
eb.es_handle.mute(False)
eb.play("/media/test8000mono.wav")
#eb.record("/media/mist.bin",100000)
time.sleep(1)
pa.set_pa_enable(False)
stm32.reset_speaker()
print("Done")

