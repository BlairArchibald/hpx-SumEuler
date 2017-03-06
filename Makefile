CXXFLAGS = -std=c++17 -g -O3 -march=native -fno-omit-frame-pointer
CXX = g++

BUILD = build

sumEuler: src/main.cpp $(BUILD)/workqueue_component.o
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp $(BUILD)/workqueue_component.o `pkg-config --cflags --libs hpx_application` -DHPX_APPLICATION_NAME=SumEuler -lhpx_iostreams

$(BUILD)/workqueue_component.o: src/workqueue_component.cpp src/workqueue_component.hpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -o $@ -c src/workqueue_component.cpp `pkg-config --cflags --libs hpx_application`
