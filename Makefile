make:
	g++ main.cpp -o a.out -lpthread -std=c++11

test:
	./a.out ./1062-prog2_data.txt
