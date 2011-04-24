proc mic_open args {
    global xh
    set xh [gemtcl_xmlrpc_client -connect -host localhost -port 8124]
}

proc mic args {
    global xh
    set cmd "gemtcl_xmlrpc_client -handle $xh -method cmd -params \"$args\""
    return [eval $cmd]
}

proc mic_image args {
    mic get_image -filename c:/temp/mic.png
    gemtcl_set image c:/temp/mic.png
}

mic_open
gemtcl_load mic.gtf
gemtcl_set dist 10
mic_image
