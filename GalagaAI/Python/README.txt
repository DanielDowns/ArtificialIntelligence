The program runs while an in-browser game of Galaga is playing. The AI takes a screenshot of the game and uses OpenCV template matching in an attempt to identify where objects in the game are and which are threats. It then uses a behavior tree to choose the next course of action and identify where to move next.

This was my first endevour into Artificial Intelligence and to be honest, was a failure (in terms of a successful product). Though the theory behind the idea was sound, the program loops too slowly to effectively adapt to the situation (even after parallelizing the program). 

However, I learned a lot about the Artificial Intelligence process and think the AI part of the implementation is correct (if naively designed).

ainodes.py defines all the types of nodes used (as well as others just for practice's sake), dodgealg.py defines how the AI ranks a point on its idealness, and main.py defines the node's execute functions and starts the program. 
