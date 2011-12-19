TARGET = build/Release/binding.node

$(TARGET): *.cc
	node-waf configure build

clean:
	node-waf distclean

.PHONY: clean
