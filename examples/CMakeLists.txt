build_lib_example(
  NAME woss-aloha-example
  SOURCE_FILES woss-aloha-example.cc
  HEADER_FILES woss-aloha-example.h
  LIBRARIES_TO_LINK
    ${libnetanim}
    ${libinternet}
    ${libmobility}
    ${libstats}
    ${libapplications}
    ${libuan}
    ${WOSS_LIBRARIES}
    ${libwoss-ns3}
)
 
