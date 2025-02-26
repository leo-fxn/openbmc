# netbase-dbg is exhibiting failures in building as the CONTROL file is emtpy.
# Remove the dbg package as netbase only include text files
PACKAGES:remove = "${PN}-dev ${PN}-staticdev ${PN}-dbg"
