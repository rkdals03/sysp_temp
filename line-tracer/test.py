import YB_Pcb_Car
import time

car = YB_Pcb_Car.YB_Pcb_Car()

car.Car_Run(150, 150)
time.sleep(2)
car.Car_Stop()

del car

