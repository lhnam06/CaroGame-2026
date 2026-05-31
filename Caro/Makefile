CXX = g++
TARGET = Caro
SRCS = main.cpp \
       Defs/Defs.cpp \
       View/View.cpp \
       Model/Model.cpp \
       Control/Control.cpp \
       SaveLoad/SaveLoad.cpp \
       Menu/Menu.cpp \
       Bot/Bot.cpp \
       Audio/Audio.cpp

all: assets
	$(CXX) -I. -std=c++17 -o $(TARGET).exe $(SRCS)

assets:
	deploy_assets.cmd . .

clean:
	del $(TARGET).exe 2>nul || true
