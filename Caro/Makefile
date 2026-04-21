CXX = g++
TARGET = Caro
SRCS = main.cpp \
       Defs/Defs.cpp \
       View/View.cpp \
       Model/Model.cpp \
       Control/Control.cpp \
       SaveLoad/SaveLoad.cpp \
       Menu/Menu.cpp \
       Bot/Bot.cpp

all:
	$(CXX) -I. -o $(TARGET).exe $(SRCS)

clean:
	del $(TARGET).exe 2>nul || true
