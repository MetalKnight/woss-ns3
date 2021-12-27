set(name woss-aloha-example)
set(source_files ${name}.cc)
set(header_files ${name}.h)
set(libraries_to_link
    ${libnetanim}
    ${libinternet}
    ${libmobility}
    ${libstats}
    ${libapplications}
    ${libuan}
    ${woss_lib}
    ${libwoss-ns3}
)
build_contrib_example(
  "${name}" "${source_files}" "${header_files}" "${libraries_to_link}"
)
