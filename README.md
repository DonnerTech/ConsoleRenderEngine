# Coral
A console game framework purely in C

## Overview
The Coral framework is built entirely in C without gpu acceleration. The framework has both a simple physics engine and a render engine that support dynamic ray traced scenes. These two parts of the framework are designed to be independant from each other so one could be used without the other. To support these features Coral also has vector, quaternion, 3x3 matrix, texture, and material datatypes with a suite of helper functions. 

## Dependancies
Coral currently relies on the following Windows specifics built into VS with the Legacy MSVC compiler:
* Win32 API
* windowscodecs lib

## Demos
Recursive RT Test:

https://github.com/user-attachments/assets/596f5b1b-5d9a-49c0-aea7-32b4d477cae5

Mats Test:

https://github.com/user-attachments/assets/94a4c640-ab16-4674-a71a-f421a8e2d442

BVH Debug View:

https://github.com/user-attachments/assets/3729bde2-40fd-4a5c-9c25-243aa3c7f4cb
