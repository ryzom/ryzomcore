ADD_SUBDIRECTORY(bnp_make)
ADD_SUBDIRECTORY(disp_sheet_id)
ADD_SUBDIRECTORY(make_sheet_id)
ADD_SUBDIRECTORY(xml_packer)

IF(WITH_QT)
  ADD_SUBDIRECTORY(words_dic_qt)
  ADD_SUBDIRECTORY(message_box_qt)
ENDIF(WITH_QT)

