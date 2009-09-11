
SOURCE_DIR 	= src
OUTPUT_DIR	= ../../static/flash
INPUT_MXML	= LDBApp.mxml
OUTPUT		= LDBApp.swf

all:
	$(FLEX_SDK_DIR)/bin/mxmlc -incremental -debug=true -load-config+=flex-config.xml src/$(INPUT_MXML) -output=$(OUTPUT_DIR)/$(OUTPUT)

