set(STB_SRC
	"stb_impl.cpp"
	)
	
set(STB_HDR
	"stb_image_write.h"
	)
	
#source_group( TREE .
#    FILES ${STB_SRC} ${STB_HDR} )

add_library(stb_image STATIC ${STB_SRC} ${STB_HDR} )
target_include_directories( stb_image PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    )
    
target_compile_features( stb_image PUBLIC cxx_std_17 )
set_target_properties( stb_image PROPERTIES CXX_EXTENSIONS OFF )
set_property(TARGET stb_image APPEND_STRING PROPERTY COMPILE_FLAGS " -w")
