from os import walk
import csv
import sys

import botNN

import keras
from keras import models, layers
import tensorflow as tf
import numpy 
import math

import dask.dataframe
import pandas as pd

import random
import time
from datetime import datetime, timedelta

STATE_ELEMENTS = 24  #number of input items, tuples expanded
ACTION_SIZE = 7 #number of output items

BATCH_SIZE = 1024
EPOCHS = 7

VALIDATION_START = .8

STEER_LOSS_THRESHOLD = 0.11 #0.125
EULER_THRESHOLD = 0.4
TIME_MINUTES_THRESHOLD = 7

optString = ""
count = 1
while count < len(sys.argv):
    optString += sys.argv[count]
    count += 1
optString = optString.replace(" ", "_")    

cleanedDirectoryString = "PATH WHERE PREPPED DATA IS"    
now = str(datetime.now())
now = now.replace(":","_").replace(" ", ".")
logDir = "WHERE YOU WANT TENSORBOARD LOGS TO LIVE"+optString+now+"\\"

tbCallback = keras.callbacks.TensorBoard(logDir, histogram_freq=1)

netBuilder = botNN.botNN(STATE_ELEMENTS)
model = netBuilder.buildModel()

print("\n") 
print("***logging at "+str(logDir))
print("\n") 

startTime = datetime.now() 
runTimes = []

files = []
for (dirpath, dirnames, filenames) in walk(cleanedDirectoryString):
    files.extend(filenames)
    break

filePaths = []
for fileName in files:
    filePaths.append(cleanedDirectoryString + "\\" + fileName)
#i have tried shuffling files. It does not seem to work :(

debugCounter = 0
checkpointCounter = 0

avgTime = []
totalStartTime = time.time()

bestAccuracy = -1
bestModel = -1
bestEuler = -1
bestLoss = -1
bestTime = -1
     
bestTimeStartMarker = time.time()
     
for file in filePaths:
    sys.stdout.flush()
    fileStartTime = time.time()
    print("training on file "+str(file))
   
    dataset = pd.read_csv(file, dtype='float64', header=None)
    
    #move dataset into state and action sets
    stateBatch = dataset.iloc[::2]
    actionBatch = dataset.iloc[1::2]
    
    #name columns to allow easy manipulation
    stateBatch.rename(columns={"A": "a", "B": "c"})
    stateBatch.columns = ['0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21','22','23']
   
    
    #normalize!
    stateBatch['1'] = stateBatch['1'].divide(4096.0)
    stateBatch['2'] = stateBatch['2'].divide(5120.0)
    stateBatch['3'] = stateBatch['3'].divide(2044.0)
    
    stateBatch['4'] = stateBatch['4'].divide((math.pi/2.0))
    stateBatch['5'] = stateBatch['5'].divide(math.pi)
    stateBatch['6'] = stateBatch['6'].divide(math.pi)
    
    stateBatch['7'] = stateBatch['7'].divide(2300.0)
    stateBatch['8'] = stateBatch['8'].divide(2300.0)
    stateBatch['9'] = stateBatch['9'].divide(2300.0)
    
    stateBatch['10'] = stateBatch['10'].divide(5.5)
    stateBatch['11'] = stateBatch['11'].divide(5.5)
    stateBatch['12'] = stateBatch['12'].divide(5.5)
    
    stateBatch['17'] = stateBatch['17'].divide(100.0)
    
    stateBatch['18'] = stateBatch['18'].divide(4096.0)
    stateBatch['19'] = stateBatch['19'].divide(5120.0)
    stateBatch['20'] = stateBatch['20'].divide(2044.0)

    stateBatch['21'] = stateBatch['21'].divide(6000.0)
    stateBatch['22'] = stateBatch['22'].divide(6000.0)
    stateBatch['23'] = stateBatch['23'].divide(6000.0)    

   
    #break apart steering and dex
    
    steerBatch = actionBatch.iloc[:, :2]   
    eulerBatch = actionBatch.iloc[:, 2:5]
    jumpBatch = actionBatch.iloc[:, 5:6]
    boostBatch = actionBatch.iloc[:, 6:7]
    

    #D-FENCE!
    assert stateBatch.shape[1] == STATE_ELEMENTS
    assert steerBatch.shape[1] == 2
    assert eulerBatch.shape[1] == 3
    assert jumpBatch.shape[1] == 1
    assert boostBatch.shape[1] == 1
    
    assert steerBatch.shape[0] == jumpBatch.shape[0]
    assert steerBatch.shape[0] == eulerBatch.shape[0]
    assert steerBatch.shape[0] == boostBatch.shape[0]
    assert steerBatch.shape[0]== stateBatch.shape[0]
    
    stateTensor = stateBatch.to_numpy()    
    steerTensor = steerBatch.to_numpy()
    eulerTensor = eulerBatch.to_numpy()
    jumpTensor = jumpBatch.to_numpy()
    boostTensor = boostBatch.to_numpy()
    
    #randomize input order with pairs still matched correctly
    rng_state = numpy.random.get_state()
    numpy.random.shuffle(stateTensor)
    numpy.random.set_state(rng_state)
    numpy.random.shuffle(steerTensor)
    numpy.random.set_state(rng_state)
    numpy.random.shuffle(eulerTensor)
    numpy.random.set_state(rng_state)
    numpy.random.shuffle(jumpTensor)
    numpy.random.set_state(rng_state)
    numpy.random.shuffle(boostTensor)
    
    #validation is last 20% of data
    validationIndexStart = math.floor(len(stateBatch)*VALIDATION_START) 
 
    #allocate validation data
    validationStates = stateTensor[validationIndexStart:]    
    validationSteer = steerTensor[validationIndexStart:]
    validationEuler = eulerTensor[validationIndexStart:]
    validationJump = jumpTensor[validationIndexStart:]
    validationBoost = boostTensor[validationIndexStart:]
    
    #remove validation for training sets
    stateTensor = stateTensor[:validationIndexStart]
    steerTensor = steerTensor[:validationIndexStart]
    eulerTensor = eulerTensor[:validationIndexStart]
    jumpTensor = jumpTensor[:validationIndexStart]
    boostTensor = boostTensor[:validationIndexStart]
 
    
        
    print("\n")    
    history = model.fit(stateTensor, 
        [steerTensor, eulerTensor, jumpTensor, boostTensor], 
        epochs=EPOCHS, 
        batch_size=BATCH_SIZE, 
        validation_data=(validationStates, [validationSteer, validationEuler, validationJump, validationBoost]),
        callbacks=[tbCallback])
        
    acc = history.history['val_steerOutput_accuracy'][-1]
    loss = history.history['val_steerOutput_loss'][-1]
    eulerAcc = history.history['val_eulerOutput_accuracy'][-1]
    
    totalElapsedMinutes = (time.time() - bestTimeStartMarker)/60
    
    if(totalElapsedMinutes > TIME_MINUTES_THRESHOLD and 
        loss < STEER_LOSS_THRESHOLD and 
        acc > bestAccuracy and 
        eulerAcc > EULER_THRESHOLD):
        
        bestAccuracy = acc
        bestModel = model
        bestEuler = eulerAcc
        bestLoss = loss
        bestTime = totalElapsedMinutes
     
    print("\n")  
    sys.stdout.flush()
    
    debugCounter = 0
    
    thisTime = time.time() - fileStartTime
    avgTime.append(thisTime)
    print("file processing time: "+str(thisTime)+" seconds")
    sys.stdout.flush()
    

print("\n")      
if(bestModel == -1):          
    print("no best model saved")     
else:
    bestModel.save_weights(logDir+"bestWeights") 
    bestModel.save(logDir+"bestModel")
    
model.save(logDir+"finalModel")
print("\n")  
print("final run loss: "+str(loss))
print("final steer accuracy: "+str(acc))
print("final euler accuracy: "+str(eulerAcc))

print("best run loss: "+str(bestLoss))
print("best steer accuracy: "+str(bestAccuracy))
print("best euler accuracy: "+str(bestEuler))
print("best time at: "+str(bestTime))

print("total run time: "+str((time.time()-totalStartTime)/60)+" minutes. Average file time: "+str(sum(avgTime) / len(avgTime))+" seconds")
