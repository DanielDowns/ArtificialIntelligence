import numpy as np
import math
import datetime
from datetime import datetime
import os
import time
import datetime

import keras
import tensorflow as tf
from agents import rlReforgedNN

TRAJ_ROUNDS = 2200 
BATCH_SIZE = 1024

MODEL_INPUT_SIZE = 30
ACTOR_LR = 1e-4
VAR_LR = 1e-4
CRITIC_LR = 1e-4
GAMMA = 0.99  
GAE_LAMBDA = 0.95
PPO_EPS = 0.1


class ppo(object):    
    def __init__(self):  

        #load NN
        builder = rlReforgedNN.reforgedNN(MODEL_INPUT_SIZE)    
        self.actorMean = builder.buildSmallModel()
        self.actorMean.load_weights(modelDir).expect_partial()  
        self.actorSteerVariance = tf.Variable(tf.zeros((2,), tf.dtypes.float32))
        self.critic = builder.buildCritic()          
        
        #set up optimizers
        self.actorOptimizer = tf.keras.optimizers.Adam(learning_rate=ACTOR_LR)
        self.varianceOptimizer = tf.keras.optimizers.Adam(learning_rate=VAR_LR)
        self.criticOptimizer = tf.keras.optimizers.Adam(learning_rate=CRITIC_LR)

        #tensorboard logging  
        tbCallback = keras.callbacks.TensorBoard(logDir, histogram_freq=1)

        #store stuff as we go
        self.stateBatch = []
        self.rewardBatch = []
        self.actionBatch = []
        self.isDoneBatch = []
        
        self.trajCount = 0
        self.totalRuns = 0
        
        
        
    def calcAdv(self, states, rewards, dones, values):
        tdList = [0] * len(states)
        lastGae = tf.Variable(tf.zeros(1), tf.float32)
        
        advantages = tf.Variable(tf.zeros(len(states), 1), tf.float32)
        references = tf.Variable(tf.zeros(len(states), 1), tf.float32)
    
        index = len(rewards) - 1       
        
        skippedFirst = False
        for val, reward, isDone in zip(reversed(values), reversed(rewards), reversed(dones)):              
            
            if (isDone == True or skippedFirst == False):            
            
                #tensorflow dumb, only way to assign to tensor
                tdList[index] = reward - val
                skippedFirst = True
                
                lastGae.assign(reward - val)
                
            else:
                #tensorflow dumb, only way to assign to tensor
                tdList[index] = reward + GAMMA * tdList[index+1] - val  
                
                delt = reward + GAMMA * tdList[index+1] - val
                lastGae.assign(delt + GAMMA * GAE_LAMBDA  * lastGae)
                
            #overly complicated syntax cause tensorflow is dumb
            advantages[index].assign(lastGae[0])    
            references[index].assign((lastGae + val)[0])
            
            index -= 1
            
        return advantages, references
        
    def calculate(self):
        if(self.trajCount >= TRAJ_ROUNDS):       
        
            #code defensively
            assert len(self.stateBatch) == len(self.rewardBatch), str(len(self.stateBatch))+" vs "+str(len(self.rewardBatch))
            assert len(self.rewardBatch) == len(self.actionBatch), str(len(self.rewardBatch))+" vs "+str(len(self.actionBatch))
            assert len(self.rewardBatch) == len(self.isDoneBatch), str(len(self.rewardBatch))+" vs "+str(len(self.isDoneBatch))
           
            statesTensor = tf.convert_to_tensor(self.stateBatch, dtype=tf.float32)
            actionTensor = tf.convert_to_tensor(self.actionBatch, dtype=tf.float32)
            rewardTensor = tf.convert_to_tensor(self.rewardBatch, dtype=tf.float32)
            doneTensor = tf.convert_to_tensor(self.isDoneBatch, dtype=tf.float32)            
            
            vStates = self.critic(statesTensor)
            advantages, references = self.calcAdv(statesTensor, rewardTensor, doneTensor, vStates)  
            advantages = (advantages - tf.math.reduce_mean(advantages)) / tf.math.reduce_std(advantages)            
            
            #generate shuffle order
            indices = tf.range(start=0, limit=tf.shape(self.stateBatch)[0], dtype=tf.int32)
            shuffledI = tf.random.shuffle(indices)
            
            #shuffle similarly
            shuffledStates = tf.gather(statesTensor, shuffledI)
            shuffledRewards = tf.gather(rewardTensor, shuffledI)
            shuffledActions = tf.gather(actionTensor, shuffledI)
            shuffledDones = tf.gather(doneTensor, shuffledI)
            shuffledReferences = tf.gather(references, shuffledI)
            shuffledAdvantages = tf.gather(advantages, shuffledI)
            
            muTensor, _2, _3, _4 = self.actorMean(shuffledStates)
            shuffledActions = tf.squeeze(shuffledActions)                  
            
            p1 = -((muTensor - shuffledActions)**2) / (2*tf.math.exp(self.actorSteerVariance))
            p2 = -tf.math.log(tf.math.sqrt(2*math.pi*tf.math.exp(self.actorSteerVariance)))
            oldPolicyVal = p1 + p2              
            
            batchNum = int(len(self.stateBatch) // BATCH_SIZE)
            for batch in range(batchNum):
                startIndex = BATCH_SIZE*batch
                
                stateMinibatch = shuffledStates[startIndex: startIndex+BATCH_SIZE]
                actionMinibatch = shuffledActions[startIndex: startIndex+BATCH_SIZE]
                rewardMinibatch = shuffledRewards[startIndex: startIndex+BATCH_SIZE]
                doneMinibatch = shuffledDones[startIndex: startIndex+BATCH_SIZE]  
                referencesMinibatch = shuffledReferences[startIndex: startIndex+BATCH_SIZE]
                advantagesMinibatch = shuffledAdvantages[startIndex: startIndex+BATCH_SIZE]  
                
                cutdownOldPolicy = oldPolicyVal[startIndex: startIndex+BATCH_SIZE]  
 
                with tf.GradientTape(persistent=True) as tape:
                    tape.watch(self.actorSteerVariance)
                    statesMiniTensor = tf.convert_to_tensor(stateMinibatch)
                    tape.watch(statesMiniTensor)
                   
                    #critic loss
                    vStates = self.critic(statesMiniTensor)
                    tape.watch(vStates)
                    
                    criticLoss = tf.math.reduce_mean(tf.math.squared_difference(vStates, referencesMinibatch))
                                  
                    #actor loss
                    muTensor, eulerTensor, jumpTensor, boostTensor = self.actorMean(statesMiniTensor)
                    
                    actionMiniTensor = tf.convert_to_tensor(actionMinibatch)
                    tape.watch(actionMiniTensor)
                    actionMiniTensor = tf.squeeze(actionMiniTensor)                    
                    
                    
                    p1 = -((muTensor - actionMiniTensor)**2) / (2*tf.math.exp(self.actorSteerVariance))#tf.clip_by_value(actorSteerVariance, 1e-5, 1e10)))
                    p2 = -tf.math.log(tf.math.sqrt(2*math.pi*tf.math.exp(self.actorSteerVariance)))
                    policyVal = p1 + p2                    
                    
                    ratio = tf.math.exp(policyVal - cutdownOldPolicy)                    
                    
                    advantagesMinibatch = tf.expand_dims(advantagesMinibatch, axis=1)
                    adv = tf.convert_to_tensor(advantagesMinibatch)
                    tape.watch(adv)
                    
                    surrObjVal = adv * ratio
                    clippedSurrVal = adv * tf.clip_by_value(ratio, 1.0 - PPO_EPS, 1.0 + PPO_EPS)
                   
                    actorLoss = -tf.math.reduce_mean(tf.math.minimum(surrObjVal, clippedSurrVal))
            
                #keep track for logging
                actorMeasure = actorLoss
                criticMeasure = criticLoss
                
                varMeasure = self.actorSteerVariance[1]
                
                actorGradients = tape.gradient(actorLoss, self.actorMean.trainable_variables)
                actorVarGradients = tape.gradient(actorLoss, self.actorSteerVariance)
                criticGradients = tape.gradient(criticLoss, self.critic.trainable_variables)    

                self.actorOptimizer.apply_gradients(zip(actorGradients, self.actorMean.trainable_variables))  
                self.varianceOptimizer.apply_gradients(zip([actorVarGradients], [self.actorSteerVariance]))
                self.criticOptimizer.apply_gradients(zip(criticGradients, self.critic.trainable_weights))
                
                del tape 
        
            #reset states
            self.stateBatch = []
            self.rewardBatch = []        
            self.actionBatch = []
            self.isDoneBatch = [] 
            
            self.trajCount = 0
            self.totalRuns += 1
