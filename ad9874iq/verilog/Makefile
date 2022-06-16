python_path = ..
verilog_files = ssi.v

all: $(verilog_files)

ssi.v: $(python_path)/ad9874ssi.py

%.v:
	python $< generate -t v > $@

clean:
	rm -rf *.v
