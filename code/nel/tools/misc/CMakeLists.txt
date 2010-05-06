SUBDIRS(bnp_make disp_sheet_id make_sheet_id xml_packer)

IF(WITH_QT)
  ADD_SUBDIRECTORY(words_dic_qt)
  ADD_SUBDIRECTORY(message_box_qt)
ENDIF(WITH_QT)

