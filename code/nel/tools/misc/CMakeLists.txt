SUBDIRS(bnp_make disp_sheet_id extract_filename lock make_sheet_id xml_packer)

IF(WITH_QT)
  ADD_SUBDIRECTORY(words_dic_qt)
  ADD_SUBDIRECTORY(message_box_qt)
ENDIF(WITH_QT)

IF(WIN32)
  ADD_SUBDIRECTORY(exec_timeout)
  ADD_SUBDIRECTORY(message_box)
  ADD_SUBDIRECTORY(multi_cd_setup_fix)

  IF(MFC_FOUND)
    ADD_SUBDIRECTORY(branch_patcher)
    ADD_SUBDIRECTORY(data_mirror)
    ADD_SUBDIRECTORY(log_analyser)
    ADD_SUBDIRECTORY(log_analyser_plug_ins)
    ADD_SUBDIRECTORY(words_dic)
  ENDIF(MFC_FOUND)
ENDIF(WIN32)

ADD_SUBDIRECTORY(probe_timers)
