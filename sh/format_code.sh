#! /bin/bash
SRC_EXT=cpp
HDR_EXT=h
CFORM_STYLE="{BasedOnStyle: Mozilla, IndentWidth: 4}"

echo "Formatting source files with style " $CFORM_STYLE
for src_file in $(find . -name "*.$SRC_EXT")
do
#	echo "Formatting" $src_file " with " $CFORM_STYLE
	clang-format -i -style="$CFORM_STYLE" $src_file
done

echo "Formatting header files with style " $CFORM_STYLE
for hdr_file in $(find . -name "*.$HDR_EXT")
do
#	echo "Formatting" $hdr_file " with " $CFORM_STYLE
	clang-format -i -style="$CFORM_STYLE" $hdr_file
done

