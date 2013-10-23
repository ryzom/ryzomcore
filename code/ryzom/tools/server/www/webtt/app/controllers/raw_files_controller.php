<?php
/*
	Ryzom Core Web-Based Translation Tool
	Copyright (C) 2011 Piotr Kaczmarek <p.kaczmarek@openlink.pl>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
?>
<?php
class RawFilesController extends AppController {

	var $name = 'RawFiles';
	var $helpers = array('Paginator', 'Time', 'Session');
	var $components = array('Session');

	function admin_index() {
		$this->RawFile->recursive = 1;
		$conditions['RawFile.dir'] = array("diff","translated");
		$this->set('rawFiles', $this->paginate($conditions));
	}

	function admin_listdir($extension = null) {
		$this->RawFile->recursive = 0;
		$this->set('rawFiles', $this->paginate(array("RawFile.extension" => $extension)));
		$this->rendeR("admin_index");
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
		$this->set('rawFile', $rawFile = $this->RawFile->find('first', array(
			"conditions" => array(
				"RawFile.dir" => $dir,
				"RawFile.filename" => $filename,
				),
					)));
		
		$this->set('fileContent', $this->RawFile->_currentFile->read());
	}

	function admin_import($dir = null, $filename = null) {
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
		$identifierModel = $translationFileModel->Identifier;
		$identifierColumnModel = $identifierModel->IdentifierColumn;
		$translationModel = $identifierModel->Translation;
		$fileIdentifierModel = $importedTranslationFileModel->FileIdentifier;

		$importedTranslationFile = $importedTranslationFileModel->find('first', array('conditions' => array('ImportedTranslationFile.filename' => $dir . DS . $filename), "recursive" => -1));
		if ($importedTranslationFile)
		{
			$this->Session->setFlash(__('Translation file already imported', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}

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
		if (!$parsedFile)
		{
			$this->Session->setFlash(__('Error importing file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}

		ini_set('max_execution_time',0);

		$processedEntities = 0;

		$importedTranslationFileModel->create();
		$data['ImportedTranslationFile']['language_id'] = $language_id;
		$data['ImportedTranslationFile']['translation_file_id'] = $translation_file_id;
		$data['ImportedTranslationFile']['filename'] = $dir . DS . $filename;
		$data['ImportedTranslationFile']['file_last_modified_date'] = $this->RawFile->_currentFileLastChange;

		$importedTranslationFileModel->saveAll($data);
		$importedTranslationFile_id = $importedTranslationFileModel->id;

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
	
			if (isset($ent['columns']) && is_array($ent['columns']) && !isset($ent['string']))
			{
				foreach ($ent['columns'] as $column_no => $value)
				{
					$ent['string'] .= $value . "\t";
				}
				$ent['string'] = substr($ent['string'], 0, -1);
			}

			if (isset($ent['diff']))
			{
				$i_data['reference_string'] = $ent['string'];
			}

			unset($identifierModel->id);
			$identifier = $identifierModel->find('first',array('conditions' => array('Identifier.identifier' => $ent['identifier'], 'Identifier.translation_file_id' => $translation_file_id), 'contain' => 'IdentifierColumn'));
			if ($identifier)
			{
				$i_data['id']=$identifier['Identifier']['id'];
			}
			else
			{
				$identifierModel->create();
				$i_data['identifier'] = $ent['identifier'];
				if (isset($ent['diff']))
					$i_data['translated'] = false;
				$newIdentifier = true;
			}
			$res = $identifierModel->saveAll($tarr = array('Identifier' => $i_data));
			$identifier_id = $identifierModel->id;

				if (!isset($ent['diff'])) // it is translated file and we add translation
				{
					unset($translationModel->id);
					unset($t_data);
					$translationHash = $translationModel->makeHash($ent);

					if ($newIdentifier) // ovbiously there's no translation for identifier we just created
						$translation = array();
					else
					{
						$this->log('new translation check');
						$this->log($translationHash);
						$translation = $translationModel->find('first',array('conditions' => array('Translation.identifier_id' => $identifier_id, 'Translation.translation_text' => $ent["string"], 'Translation.translation_hash' => NULL), "recursive" => -1));
						$this->log($translation);
						if (!$translation)
						{
							$translation = $translationModel->find('first',array('conditions' => array('Translation.identifier_id' => $identifier_id, 'Translation.translation_hash' => $translationHash), "recursive" => -1));
							$this->log($translation);
						}
						$this->log('new translation check end');
					}

					if (!$translation)
					{
						$this->log('new translation');
						$t_data['identifier_id'] = $identifier_id;
						$t_data['translation_text'] = $ent['string'];
						$t_data['user_id'] = $this->Auth->user('id');
						$t_data['translation_hash'] = $translationHash;
						$translationModel->save(array('Translation' => $t_data));
					}
/*					else
						$t_data['id'] = $translation['Translation']['id'];*/
					if ($translation)
						$parentTranslation_id = $translation['Translation']['id'];
					else
						$parentTranslation_id = $translationModel->id;
				}

			if (isset($ent['columns']) && is_array($ent['columns']))
			{
				$ic_data = array();
				foreach ($ent['columns'] as $column_no => $value)
				{
					unset($identifierColumnModel->id);
					$ic_arr = array();
					$ic_arr['identifier_id'] = $identifier_id;
					$column_name = $_columns[$column_no];
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
					$res = $identifierColumnModel->save($ic_arr);
					$identifierColumn_id = $identifierColumnModel->id;
					
					if (!isset($ent['diff'])) // it is translated file and we add translation
					{
						unset($translationModel->id);
						unset($t_data);
						if ($newIdentifier) // ovbiously there's no translation for identifier we just created
							$translation = array();
						else
							$translation = $translationModel->find('first',array('conditions' => array('Translation.identifier_column_id' => $identifierColumn_id, 'Translation.translation_text' => $value, 'Translation.parent_id' => $parentTranslation_id), "recursive" => -1));

						if (!$translation)
						{
							$t_data['identifier_column_id'] = $identifierColumn_id;
							$t_data['translation_text'] = $value;
							$t_data['user_id'] = $this->Auth->user('id');
							$t_data['parent_id'] = $parentTranslation_id;
							$translationModel->save(array('Translation' => $t_data));
						}
/*						else
							$t_data['id'] = $translation['Translation']['id'];*/
					}
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
				$fi_data['identifier_id'] = $identifier_id;

				$res = $fileIdentifierModel->saveAll($tarr = array('FileIdentifier' => $fi_data));
				$fileIdentifier_id = $fileIdentifierModel->id;
			}

			$processedEntities++;

		}
		if ($processedEntities == 0)
		{
			$importedTranslationFileModel->delete($importedTranslationFile_id);
			$this->Session->setFlash(__('File was not imported because it seems empty', true));
			$this->redirect($this->referer());
			return 0;
		}
		else
		{
			$this->Session->setFlash(__('Translation file imported into database successfully. Processed entities: ' . $processedEntities, true));
			$this->redirect(array('controller' => 'imported_translation_files', 'action' => 'view', $importedTranslationFileModel->id));
			return 0;
		}
	}

	function admin_export($dir = null, $filename = null, $importedTranslationFileId = null) {
		if (!$filename) {
			$this->Session->setFlash(__('Invalid file', true));
			$this->redirect($this->referer());
			return 0;
		}
		if (!$this->RawFile->open($dir, $filename, $writable = true))
		{
			$this->Session->setFlash(__('Can\'t open file for writing', true));
			$this->redirect($this->referer());
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
			)
		);
		if (!$importedTranslationFile)
		{
			$this->Session->setFlash(__('No imported translation file found for chosen file', true));
			$this->redirect($this->referer());
			return 0;
		}
		
		$translationFileModel = $importedTranslationFileModel->TranslationFile;
		$identifierModel = $translationFileModel->Identifier;

		// check if all identifiers have "best" translation
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
				'identifier' => $fileIdentifier['Identifier']['identifier'],
				'arguments' => ((isset($fileIdentifier['arguments']) && !empty($fileIdentifier['arguments'])) ? $fileIdentifier['arguments'] : null),
			);
			if (isset($fileIdentifier['Identifier']['Translation'][0]))
			{
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

		ini_set('max_execution_time',0);

		$result = $this->RawFile->buildFile($entities);
		if (!$result)
		{
			$this->Session->setFlash(__('Error exporting file', true));
			$this->redirect(array('action' => 'index'));
			return 0;
		}
	}
}
