main: generate-table
	./generate-table

generate-table: *.cpp *.h
	clang++ generate_table.cpp  pokerlib.cpp mtrand.cpp -o ./generate-table
