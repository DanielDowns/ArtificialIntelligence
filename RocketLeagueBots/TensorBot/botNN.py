from keras import models, layers, Input, Model
import keras
import tensorflow as tf


class botNN:
    def __init__(self, inputSize):
        self.inSize = inputSize


#working ideas: 
#ignore angular velocity as input
#is 512->128 a bottleneck?
#training data choices (Kamael vs Rookie, etc)
#running with GUI open vs minimized
#---seems to be roughly the same? 
        
    def buildModel(self):
        lrelu = lambda x: tf.keras.activations.relu(x, alpha=0.1)
    
        inputLayer = Input(shape=(self.inSize,), name="inputLayer")
        archLayer = layers.Dense(512, activation=lrelu)(inputLayer)
        archLayer = layers.Dense(512, activation=lrelu)(inputLayer)        

        steerModel = layers.Dense(512, activation=lrelu)(archLayer)
        steerModel = layers.Dense(512, activation=lrelu)(steerModel)
        steerModel = layers.Dense(512, activation=lrelu)(steerModel)
        steerModel = layers.Dense(512, activation=lrelu)(steerModel)
        steerOutput = layers.Dense(2, name='steerOutput')(steerModel)
       
        eulerModel = layers.Dense(128, activation=lrelu)(archLayer)
        eulerModel = layers.Dense(256, activation=lrelu)(eulerModel)
        eulerModel = layers.Dense(512, activation=lrelu)(eulerModel)
        eulerModel = layers.Dense(512, activation=lrelu)(eulerModel)
        eulerOutput = layers.Dense(3, name = "eulerOutput")(eulerModel)
        
        jumpModel = layers.Dense(128, activation='relu')(archLayer)
        jumpModel = layers.Dense(256, activation='relu')(jumpModel)
        jumpOutput = layers.Dense(1, activation='sigmoid', name="jumpOutput")(jumpModel)
        
        boostModel = layers.Dense(128, activation='relu')(archLayer)
        boostModel = layers.Dense(256, activation='relu')(boostModel)
        boostOutput = layers.Dense(1, activation='sigmoid', name = "boostOutput")(boostModel)        

        model = Model(inputLayer, [steerOutput, eulerOutput, jumpOutput, boostOutput])
    
        opt = keras.optimizers.Adam(learning_rate=0.001)
        
        #*****loss will probably need to be weighted
        model.compile(optimizer=opt, 
            loss=['mse', 'mse', 'binary_crossentropy', 'binary_crossentropy'], 
            metrics=['accuracy', 'mse', 'binary_crossentropy'])
        model.summary()
        return model

    