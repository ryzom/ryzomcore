<?php
class RawFilesController extends AppController {

	var $name = 'RawFiles';
	var $helpers = array('Paginator', 'Time', 'Session');
	var $components = array('Session');

	function admin_index() {
		$this->RawFile->recursive = 1;
//		var_dump($this->RawFile->find('count'));
//		$db =& ConnectionManager::getDataSource($this->RawFile->useDbConfig);
//		var_dump($db->calculate($this->RawFile, 'count'));
		$conditions['RawFile.dir'] = array("diff","translated");
		$this->set('rawFiles', $this->paginate($conditions));
//		$this->log(Router::parse($this->referer()));
//		var_dump($this->paginate());
	}

	function admin_listdir($extension = null) {
		$this->RawFile->recursive = 0;
		$this->set('rawFiles', $this->paginate(array("RawFile.extension" => $extension)));
		$this->rendeR("admin_index");
//		var_dump($this->paginate());
	}

	function admin_view($dir = null, $filename = null) {
		if (!$filename) {
			$this->Session->setFlash(__('Invalid raw file', true));
			$this->redirect(array('action' => 'index'));
		}
		if (!$this->RawFile->open($dir, $filename))
		{
			$this->Session->setFlash(__('Can\'t open file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
//		$id = $dir . DS . $filename;
//		$this->set('rawFile', $this->RawFile->read(null, $id));
		$this->set('rawFile', $rawFile = $this->RawFile->find('first', array(
			"conditions" => array(
				"RawFile.dir" => $dir,
				"RawFile.filename" => $filename,
				),
					)));
		
		$this->set('fileContent', $this->RawFile->_currentFile->read());
	}

/*	function import($dir = null, $filename = null) {
		$this->admin_import($dir, $filename);
	}*/

	function admin_import($dir = null, $filename = null) {
//		$this->view = "index";
//		App::import("Vendor","UxtParser", array("file" => 'UxtParser.php'));
		if (!$filename) {
			$this->Session->setFlash(__('Invalid file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		if (!$this->RawFile->open($dir, $filename))
		{
			$this->Session->setFlash(__('Can\'t open file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		$importedTranslationFileModel = $this->RawFile->ImportedTranslationFile;
		$translationFileModel = $importedTranslationFileModel->TranslationFile;
		$languageModel = $translationFileModel->Language;
//		$identifierModel = $languageModel->Identifier;
		$identifierModel = $translationFileModel->Identifier;
		$identifierColumnModel = $identifierModel->IdentifierColumn;
		$translationModel = $identifierModel->Translation;
		$fileIdentifierModel = $importedTranslationFileModel->FileIdentifier;

//		$filename="diff/pl_diff_4DEC868A.uxt";
		$importedTranslationFile = $importedTranslationFileModel->find('first', array('conditions' => array('ImportedTranslationFile.filename' => $dir . DS . $filename), "recursive" => -1));
/*		var_dump($translationFile);
		return 0;*/
		if ($importedTranslationFile)
		{
			$this->Session->setFlash(__('Translation file already imported', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
//		var_dump($file);
//		$parser = new UxtParser();
//		$arr = explode("_", basename($filename, ".uxt"));
//		var_dump($arr);

//		$language_id = 1;
		$languageCode = $this->RawFile->getLanguageCode($filename);
		if (!$languageCode)
		{
			$this->Session->setFlash(__('Can\'t identify language', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		$language = $languageModel->find('first', array('conditions' => array('code' => $languageCode), "recursive" => -1));
		$language_id = $language['Language']['id'];
		if (!$language_id)
		{
			$this->Session->setFlash(__('Can\'t find language in database', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
		else
		{
//			var_dump($language_id);
		}

		$filename_template = preg_replace('/_diff/', '', $filename);
		$filename_template = preg_replace('/_[A-F0-9]{8}/', '', $filename_template);

//		for global identifiers
/*		if (preg_match('|^.*_' . $language['Language']['code'] . '.*$|', $filename_template, $matches))
			$filename_template = preg_replace('/_' . $language['Language']['code'] . '/', '_LC', $filename_template);
		else if (preg_match('|^.*' . $language['Language']['code'] . '.*$|', $filename_template, $matches))
			$filename_template = preg_replace('/' . $language['Language']['code'] . '/', 'LC', $filename_template);
		else
		{
			$this->Session->setFlash(__('Can\'t create master translation filename template from current filename', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}*/
		
		$translationFile = $translationFileModel->find('first', array('conditions' => array('filename_template' => $filename_template), "recursive" => -1));
		if (!$translationFile)
		{
			$tf_data['filename_template'] = $filename_template;
			$tf_data['language_id'] = $language_id;
		}
		else
			$tf_data['id'] = $translationFile['TranslationFile']['id'];

		$res = $translationFileModel->saveAll(array('TranslationFile' => $tf_data));
		$translation_file_id = $translationFileModel->id;

		$parsedFile = $this->RawFile->parseFile();
/*		var_dump($parsedFile);
		$this->render('index');
		return 0;*/
//		$this->log($parsedFile);
		if (!$parsedFile)
		{
			$this->Session->setFlash(__('Error importing file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
//		$this->log($parsedFile);
//		return 0;

		ini_set('max_execution_time',0);

		$processedEntities = 0;

		$importedTranslationFileModel->create();
		$data['ImportedTranslationFile']['language_id'] = $language_id;
		$data['ImportedTranslationFile']['translation_file_id'] = $translation_file_id;
		$data['ImportedTranslationFile']['filename'] = $dir . DS . $filename;
		$data['ImportedTranslationFile']['file_last_modified_date'] = $this->RawFile->_currentFileLastChange;

//			$data['TranslationFile'] = $tf_data;
		$importedTranslationFileModel->saveAll($data);
		$importedTranslationFile_id = $importedTranslationFileModel->id;

		//$this->ImportedTranslationFile->save($data);
		foreach ($parsedFile as $ent)
		{
			if (!isset($ent['type']))
				var_dump($ent);

			if ($ent['type'] == 'sheet_description')
			{
				$_columns = $ent['columns'];
				$_sheet_id_column = $ent['sheet_id_column'];
			}

			if ($ent['type'] != "string" && $ent['type'] != "phrase" && $ent['type'] != 'sheet')
				continue;

			$newIdentifier = false;
			$i_data = array();
			$i_data['language_id'] = $language_id;
			$i_data['translation_file_id'] = $translation_file_id;
			if (isset($ent['index']))
				$i_data['translation_index'] = $ent['index'];
			if (isset($ent['arguments']))
				$i_data['arguments'] = $ent['arguments'];
	
			if (isset($ent['diff']) && isset($ent['string']))
			{
				$i_data['reference_string'] = $ent['string'];
			}

			unset($identifierModel->id);
			$identifier = $identifierModel->find('first',array('conditions' => array('Identifier.identifier' => $ent['identifier'], 'Identifier.translation_file_id' => $translation_file_id), 'contain' => 'IdentifierColumn'));
			//App::import('Vendor', 'DebugKit.FireCake');
//			FireCake::log($ent['identifier'], "Identifier");
//			FireCake::dump("identifier",$identifier);
/*			$this->log($ent['identifier']);
			$this->log($identifier);*/
			if ($identifier)
			{
//				var_dump($identifier);
				$i_data['id']=$identifier['Identifier']['id'];
//				$this->log("found");
			}
			else
			{
				$identifierModel->create();
				$i_data['identifier'] = $ent['identifier'];
				if (isset($ent['diff']))
					$i_data['translated'] = false;
				$newIdentifier = true;
//				$this->log("not found");
//				$this->log("id: # " . $identifierModel->id . " #");
//				$this->log($i_data);

			}
//			var_dump($i_data);
			$res = $identifierModel->saveAll($tarr = array('Identifier' => $i_data));
			$identifier_id = $identifierModel->id;
/*			$this->log('identifier saveAll res');
			$this->log($res);
			$this->log(var_export($res,true));
			$this->log($identifierModel->validationErrors);
			$this->log($identifierModel);
			$this->log('#identifier id');
			$this->log($identifier_id);
			$this->log("tarr");
			$this->log($tarr);*/

			if (isset($ent['columns']) && is_array($ent['columns']))
			{
/*				$this->log($_columns);
				$this->log($ent['columns']);*/
				$ic_data = array();
				foreach ($ent['columns'] as $column_no => $value)
				{
					unset($identifierColumnModel->id);
					$ic_arr = array();
					$ic_arr['identifier_id'] = $identifier_id;
					$column_name = $_columns[$column_no];
/*					$this->log($identifier);
					$this->log($column_name);*/
					if (!$newIdentifier)
					{
						foreach ($identifier['IdentifierColumn'] as $identifierColumn_no => $identifierColumn)
						{
							if ($identifierColumn['column_name'] == $column_name)
							{
								$ic_arr['id'] = $identifierColumn['id'];
								break;
							}
						}
					}
					$ic_arr['column_name'] = $column_name;
					if (isset($ent['diff']))
						$ic_arr['reference_string'] = $value;
					$ic_data[] = $ic_arr;
//					$this->log($ic_arr);
					$res = $identifierColumnModel->save($ic_arr);
//					$this->log($res);
					$identifierColumn_id = $identifierColumnModel->id;
/*					$this->log($identifierColumnModel->validationErrors);
					$this->log(var_export($res,true));*/
					
					if (!isset($ent['diff'])) // it is translated file and we add translation
					{
						unset($translationModel->id);
						if ($newIdentifier) // ovbiously there's no translation for identifier we just created
							$translation = array();
						else
							$translation = $translationModel->find('first',array('conditions' => array('Translation.identifier_column_id' => $identifierColumn_id, 'Translation.translation_text' => $value), "recursive" => -1));

						if (!$translation)
						{
							$t_data['identifier_column_id'] = $identifierColumn_id;
							$t_data['translation_text'] = $value;
							// TODO: change user_id for authorized user
							$t_data['user_id'] = 1;
						}
						else
							$t_data['id'] = $translation['Translation']['id'];
//						var_dump($i_data);
						$translationModel->save(array('Translation' => $t_data));
					}

				}
/*				$res = $identifierColumnModel->saveAll($tarr = array('IdentifierColumn' => $ic_data));
				$this->log($tarr);
				$this->log(var_export($res,true));
				$this->log($identifierColumnModel->validationErrors);*/
			}
			else
			{
				if (!isset($ent['diff'])) // it is translated file and we add translation
				{
					unset($translationModel->id);
					if ($newIdentifier) // ovbiously there's no translation for identifier we just created
						$translation = array();
					else
						$translation = $translationModel->find('first',array('conditions' => array('Translation.identifier_id' => $identifier_id, 'Translation.translation_text' => $ent["string"]), "recursive" => -1));
					if (!$translation)
					{
						$t_data['identifier_id'] = $identifier_id;
						if (isset($ent['string'])) // sheets doesn't have string (they have columns)
							$t_data['translation_text'] = $ent['string'];
						// TODO: change user_id for authorized user
						$t_data['user_id'] = 1;
					}
					else
						$t_data['id'] = $translation['Translation']['id'];
//					var_dump($i_data);
					$translationModel->save(array('Translation' => $t_data));
				}
			}

			unset($fileIdentifierModel->id);
			$fi_data = array();
			$fi_data['imported_translation_file_id'] = $importedTranslationFile_id;
			// TOTHINK - set FileIdentifier['id'] if we import already imported file (not supporting importing imported file)
//			$identifier = $this->ImportedTranslationFile->FileIdentifier->find('first',array('conditions' => array('FileIdentifier.identifier' => $ent['identifier'], 'FileIdentifier.translation_file_id' => $)));
//			$data['FileIdentifier']['translation_file_id'] = $this->ImportedTranslationFile->id;
			if (isset($ent['arguments']))
				$fi_data['arguments'] = $ent['arguments'];

			if (isset($_sheet_id_column))
				$fi_data['arguments'] = $_sheet_id_column;

			if (isset($ent['diff'])) // it is diff file
			{
				if (isset($ent['command']))
					$fi_data['command'] = $ent['command'];
				else
					$fi_data['command'] = "DIFF " . mb_strtoupper($ent['diff']);

				if (isset($ent['string']))
					$fi_data['reference_string'] = $ent['string'];

				if (isset($ent['index']))
					$fi_data['translation_index'] = $ent['index'];
//				$data['FileIdentifier']['identifier_id'] = ;
				$fi_data['identifier_id'] = $identifier_id;

				$res = $fileIdentifierModel->saveAll($tarr = array('FileIdentifier' => $fi_data));
//				$this->log($res);
/*				$this->log("#fi_data");
				$this->log($fi_data);*/
				$fileIdentifier_id = $fileIdentifierModel->id;
			}

//			$this->ImportedTranslationFile->FileIdentifier->create();
//			$this->ImportedTranslationFile->FileIdentifier->save($data);
//			$data['FileIdentifier'][] = $fi_data;
//			$l_data['Language']['id'] = $language_id;
//			$l_data['Identifier'][] = $i_data;
//			$data['Identifier'][] = $i_data;

			$processedEntities++;

		}
/*		$this->render('admin_index');
		return 0;*/
//		var_dump($data);
//		$this->ImportedTranslationFile->Language->saveAll($l_data);
		if ($processedEntities == 0)
		{
			$importedTranslationFileModel->delete($importedTranslationFile_id);
			$this->Session->setFlash(__('File was not imported because it seems empty', true));
//			$this->redirect(array('action' => 'index'));
			$this->redirect($this->referer());
			return 0;
		}
		else
		{
			$this->Session->setFlash(__('Translation file imported into database successfully. Processed entities: ' . $processedEntities, true));
			$this->redirect(array('controller' => 'imported_translation_files', 'action' => 'view', $importedTranslationFileModel->id));
//			$this->render('admin_index');
			return 0;
		}
//		$this->ImportedTranslationFile->recursive = 0;
//		$this->set('importedTranslationFiles', $this->paginate());
//		$this->render('index');
	}

	function admin_export($dir = null, $filename = null, $importedTranslationFileId = null) {
		if (!$filename) {
			$this->Session->setFlash(__('Invalid file', true));
			$this->redirect($this->referer());
//			$this->redirect(array('action' => 'index'));
			return 0;
		}
		if (!$this->RawFile->open($dir, $filename, $writable = true))
		{
			$this->Session->setFlash(__('Can\'t open file for writing', true));
			$this->redirect($this->referer());
//			$this->redirect(array('action' => 'index'));
			return 0;
		}

		$importedTranslationFileModel = $this->RawFile->ImportedTranslationFile;

		$importedTranslationFileModel->contain(array(
			'TranslationFile',
			'FileIdentifier' => array('Identifier' => array(
				'Translation',
				'IdentifierColumn' => 'Translation',
				)),
		));

		$importedTranslationFile = $importedTranslationFileModel->find('first', array(
			'conditions' => array(
				'ImportedTranslationFile.filename' => $dir . DS . $filename
				),
//			'recursive' => 3
//			'order' => 'FileIdentifier.translation_index',
			)
		);
/*		var_dump($translationFile);
		return 0;*/
		if (!$importedTranslationFile)
		{
			$this->Session->setFlash(__('No imported translation file found for chosen file', true));
			$this->redirect($this->referer());
//			$this->redirect(array('controller' => 'imported_translation_files', 'action' => 'index'));
			return 0;
		}
		
		$translationFileModel = $importedTranslationFileModel->TranslationFile;
		$identifierModel = $translationFileModel->Identifier;

		// TODO: check if all identifiers have "best" translation
		$identifier_ids = $identifierModel->withoutBestTranslation(array('ImportedTranslationFile.id' => $importedTranslationFile['ImportedTranslationFile']['id']));
		if ($identifier_ids === false)
		{
			$this->Session->setFlash(__('Error: no conditions specified', true));
			$this->redirect($this->referer());
			return 0;
		}
		else if (count($identifier_ids) > 0)
		{
			$this->Session->setFlash(__('Best translation is not set for some of the identifiers in this file. Set best translation before export.', true));
			$this->redirect(array('controller' => 'identifiers', 'action' => 'withoutBestTranslation', 'imported_translation_file_id' => $importedTranslationFile['ImportedTranslationFile']['id']));
			return 0;
		}

		$translationFile_id = $importedTranslationFile['ImportedTranslationFile']['translation_file_id'];

		$i=0;
		$sortResult = Set::sort($importedTranslationFile['FileIdentifier'], '{n}.translation_index', 'asc');
		if (!$sortResult)
		{
			$this->Session->setFlash(__('Sorting error', true));
			$this->redirect($this->referer());
			return 0;
		}

		foreach ($sortResult as $fileIdentifier)
		{
			if ($fileIdentifier['Identifier']['IdentifierColumn'] && !isset($entities[0]))
			{
				foreach ($fileIdentifier['Identifier']['IdentifierColumn'] as $column_no => $identifierColumn)
					$_columns[$column_no] = $identifierColumn['column_name'];
				$ent['columns'] = $_columns;
				$ent['type'] = 'sheet_description';
				$ent['sheet_id_column'] = $fileIdentifier['arguments'];
				$ent['diff'] = ((isset($fileIdentifier['command']) && !empty($fileIdentifier['command'])) ? true : null);
				$entities[] = $ent;
				$ent = array();
			}
			$ent = array(
				'diff' => ((isset($fileIdentifier['command']) && !empty($fileIdentifier['command'])) ? $fileIdentifier['command'] : null),
				'command' => ((isset($fileIdentifier['command']) && !empty($fileIdentifier['command'])) ? $fileIdentifier['command'] : null),
				'index' => ((isset($fileIdentifier['translation_index']) && !empty($fileIdentifier['translation_index'])) ? $fileIdentifier['translation_index'] : null),
				'internal_index' => $i++,
				'type' => ((count($fileIdentifier['Identifier']['IdentifierColumn']) > 0) ? 'sheet' : 'string'),
//				'type' => ((isset($fileIdentifier['command']) && !empty($fileIdentifier['command'])) ? $fileIdentifier['command'] : null),
				'identifier' => $fileIdentifier['Identifier']['identifier'],
				'arguments' => ((isset($fileIdentifier['arguments']) && !empty($fileIdentifier['arguments'])) ? $fileIdentifier['arguments'] : null),
//				'string' => '',
			);
//			$this->log($fileIdentifier['Identifier']['Translation']);
//			if (Set::numeric(array_keys($fileIdentifier['Identifier']['Translation'])))
			if (isset($fileIdentifier['Identifier']['Translation'][0]))
			{
//				$this->log('numeric');
				$ent['string'] = $fileIdentifier['Identifier']['Translation'][0]['translation_text'];
			}
			else if (isset($fileIdentifier['Identifier']['Translation']['translation_text']))
				$ent['string'] = $fileIdentifier['Identifier']['Translation']['translation_text'];

			if (isset($fileIdentifier['Identifier']['BestTranslation']['translation_text']))
				$ent['string'] = $fileIdentifier['Identifier']['BestTranslation']['translation_text'];

			if (($export_reference_if_empty_translation = true) && !isset($ent['string']))
				$ent['string'] = $fileIdentifier['Identifier']['reference_string'];
			else if (!isset($ent['string']))
				$ent['string'] = '';

			foreach ($fileIdentifier['Identifier']['IdentifierColumn'] as $column_no => $identifierColumn)
			{
/*				if (isset($identifierColumn['Translation']['translation_text']))
					$ent['columns'][$column_no] = $identifierColumn['Translation']['translation_text'];*/

				if (isset($identifierColumn['Translation'][0]))
					$ent['columns'][$column_no] = $identifierColumn['Translation'][0]['translation_text'];
				else if (isset($identifierColumn['Translation']['translation_text']))
					$ent['columns'][$column_no] = $identifierColumn['Translation']['translation_text'];

				if (isset($identifierColumn['BestTranslation']['translation_text']))
					$ent['columns'][$column_no] = $identifierColumn['BestTranslation']['translation_text'];

				if ($export_reference_if_empty_translation && !isset($ent['columns'][$column_no]))
					$ent['columns'][$column_no] = $identifierColumn['reference_string'];
				else if (!isset($ent['columns'][$column_no]))
					$ent['columns'][$column_no] = '';
			}
			if ($fileIdentifier['command'])
				$ent['command'] = $ent['diff'] = $fileIdentifier['command'];

			$entities[] = $ent;
		}
/*		$sources = ConnectionManager::sourceList();
		$sqlLogs = array();
		foreach ($sources as $source)
		{
			$db =& ConnectionManager::getDataSource($source);
			if (!$db->isInterfaceSupported('getLog'))
				continue;
			$sqlLogs[$source] = $db->getLog();
		}
		$this->log($sqlLogs);*/

//		$this->log($importedTranslationFile);
/*		$this->log($sortResult);
		$this->log($entities);*/

		ini_set('max_execution_time',0);

		$result = $this->RawFile->buildFile($entities);
//		$this->log($result);
//			$this->render('admin_index');
//			$this->redirect(array('controller' => 'imported_translation_files', 'action' => 'index'));
//			return 0;

/*		var_dump($parsedFile);
		$this->render('index');
		return 0;*/
//		$this->log($parsedFile);
		if (!$result)
		{
			$this->Session->setFlash(__('Error exporting file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
	}
}
