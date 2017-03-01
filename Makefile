CXXFLAGS = -std=c++17 -g -O3 -march=native -fno-omit-frame-pointer
CXX = g++

sumEuler: src/main.cpp src/workqueue_component.cpp
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp src/workqueue_component.cpp `pkg-config --cflags --libs hpx_application` -DHPX_APPLICATION_NAME=SumEuler -lhpx_iostreams
