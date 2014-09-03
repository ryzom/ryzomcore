<?php
/**
* This function is beign used to load info that's needed for the settings page.
* check if the person who wants to view this page is a mod/admin or the user to whom te settings belong himself, if this is not the case, he will be redirected to an error page.
* it will return a lot of information of that user, that's being used for loading the template.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function settings(){
    if(WebUsers::isLoggedIn()){
            //in case id-GET param set it's value as target_id, if no id-param is given, ue the session id.
            if(isset($_GET['id'])){
                if(($_GET['id'] != $_SESSION['id']) && (!Ticket_User::isMod(unserialize($_SESSION['ticket_user']))) ){
                    //ERROR: No access!
                    $_SESSION['error_code'] = "403";
                    header("Location: index.php?page=error");
                    throw new SystemExit();
                }else{
                    $webUser = new Webusers($_GET['id']);
                    $result = $webUser->getInfo();
                    if(Ticket_User::isMod(unserialize($_SESSION['ticket_user'])) && ($_GET['id']!= $_SESSION['id'])){
                        $result['changesOther'] = "TRUE";
                    }
                    $result['target_id'] = $_GET['id'];
                    $result['current_mail'] = $webUser->getEmail();
                    $result['target_username'] = $webUser->getUsername();
                }
            }else{
                $webUser = new Webusers($_SESSION['id']);
                $result = $webUser->getInfo();
                $result['target_id'] = $_SESSION['id'];
                $result['current_mail'] = $webUser->getEmail();
                $result['target_username'] = $webUser->getUsername();

            }
            //Sanitize Data
            $result['current_mail'] = filter_var($result['current_mail'], FILTER_SANITIZE_EMAIL);
            $result['target_username'] = filter_var($result['target_username'], FILTER_SANITIZE_STRING);
            $result['FirstName'] = filter_var($result['FirstName'], FILTER_SANITIZE_STRING);
            $result['LastName'] = filter_var($result['LastName'], FILTER_SANITIZE_STRING);
            $result['Country'] = filter_var($result['Country'], FILTER_SANITIZE_STRING);
            $result['Gender'] = filter_var($result['Gender'], FILTER_SANITIZE_NUMBER_INT);
            $result['ReceiveMail'] = filter_var($result['ReceiveMail'], FILTER_SANITIZE_NUMBER_INT);
            $result['country_array'] = getCountryArray();
            global $INGAME_WEBPATH;
            $result['ingame_webpath'] = $INGAME_WEBPATH;
            return $result;
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        throw new SystemExit();
    }
}


function getCountryArray(){

$countries = array(
'AA'=>'None Selected',
'AF'=>'Afghanistan',
'AL'=>'Albania',
'DZ'=>'Algeria',
'AS'=>'American Samoa',
'AD'=>'Andorra',
'AO'=>'Angola',
'AI'=>'Anguilla',
'AQ'=>'Antarctica',
'AG'=>'Antigua And Barbuda',
'AR'=>'Argentina',
'AM'=>'Armenia',
'AW'=>'Aruba',
'AU'=>'Australia',
'AT'=>'Austria',
'AZ'=>'Azerbaijan',
'BS'=>'Bahamas',
'BH'=>'Bahrain',
'BD'=>'Bangladesh',
'BB'=>'Barbados',
'BY'=>'Belarus',
'BE'=>'Belgium',
'BZ'=>'Belize',
'BJ'=>'Benin',
'BM'=>'Bermuda',
'BT'=>'Bhutan',
'BO'=>'Bolivia',
'BA'=>'Bosnia And Herzegovina',
'BW'=>'Botswana',
'BV'=>'Bouvet Island',
'BR'=>'Brazil',
'IO'=>'British Indian Ocean Territory',
'BN'=>'Brunei',
'BG'=>'Bulgaria',
'BF'=>'Burkina Faso',
'BI'=>'Burundi',
'KH'=>'Cambodia',
'CM'=>'Cameroon',
'CA'=>'Canada',
'CV'=>'Cape Verde',
'KY'=>'Cayman Islands',
'CF'=>'Central African Republic',
'TD'=>'Chad',
'CL'=>'Chile',
'CN'=>'China',
'CX'=>'Christmas Island',
'CC'=>'Cocos (Keeling) Islands',
'CO'=>'Columbia',
'KM'=>'Comoros',
'CG'=>'Congo',
'CK'=>'Cook Islands',
'CR'=>'Costa Rica',
'CI'=>'Cote D\'Ivorie (Ivory Coast)',
'HR'=>'Croatia (Hrvatska)',
'CU'=>'Cuba',
'CY'=>'Cyprus',
'CZ'=>'Czech Republic',
'CD'=>'Democratic Republic Of Congo (Zaire)',
'DK'=>'Denmark',
'DJ'=>'Djibouti',
'DM'=>'Dominica',
'DO'=>'Dominican Republic',
'TP'=>'East Timor',
'EC'=>'Ecuador',
'EG'=>'Egypt',
'SV'=>'El Salvador',
'GQ'=>'Equatorial Guinea',
'ER'=>'Eritrea',
'EE'=>'Estonia',
'ET'=>'Ethiopia',
'FK'=>'Falkland Islands (Malvinas)',
'FO'=>'Faroe Islands',
'FJ'=>'Fiji',
'FI'=>'Finland',
'FR'=>'France',
'FX'=>'France, Metropolitan',
'GF'=>'French Guinea',
'PF'=>'French Polynesia',
'TF'=>'French Southern Territories',
'GA'=>'Gabon',
'GM'=>'Gambia',
'GE'=>'Georgia',
'DE'=>'Germany',
'GH'=>'Ghana',
'GI'=>'Gibraltar',
'GR'=>'Greece',
'GL'=>'Greenland',
'GD'=>'Grenada',
'GP'=>'Guadeloupe',
'GU'=>'Guam',
'GT'=>'Guatemala',
'GN'=>'Guinea',
'GW'=>'Guinea-Bissau',
'GY'=>'Guyana',
'HT'=>'Haiti',
'HM'=>'Heard And McDonald Islands',
'HN'=>'Honduras',
'HK'=>'Hong Kong',
'HU'=>'Hungary',
'IS'=>'Iceland',
'IN'=>'India',
'ID'=>'Indonesia',
'IR'=>'Iran',
'IQ'=>'Iraq',
'IE'=>'Ireland',
'IL'=>'Israel',
'IT'=>'Italy',
'JM'=>'Jamaica',
'JP'=>'Japan',
'JO'=>'Jordan',
'KZ'=>'Kazakhstan',
'KE'=>'Kenya',
'KI'=>'Kiribati',
'KW'=>'Kuwait',
'KG'=>'Kyrgyzstan',
'LA'=>'Laos',
'LV'=>'Latvia',
'LB'=>'Lebanon',
'LS'=>'Lesotho',
'LR'=>'Liberia',
'LY'=>'Libya',
'LI'=>'Liechtenstein',
'LT'=>'Lithuania',
'LU'=>'Luxembourg',
'MO'=>'Macau',
'MK'=>'Macedonia',
'MG'=>'Madagascar',
'MW'=>'Malawi',
'MY'=>'Malaysia',
'MV'=>'Maldives',
'ML'=>'Mali',
'MT'=>'Malta',
'MH'=>'Marshall Islands',
'MQ'=>'Martinique',
'MR'=>'Mauritania',
'MU'=>'Mauritius',
'YT'=>'Mayotte',
'MX'=>'Mexico',
'FM'=>'Micronesia',
'MD'=>'Moldova',
'MC'=>'Monaco',
'MN'=>'Mongolia',
'MS'=>'Montserrat',
'MA'=>'Morocco',
'MZ'=>'Mozambique',
'MM'=>'Myanmar (Burma)',
'NA'=>'Namibia',
'NR'=>'Nauru',
'NP'=>'Nepal',
'NL'=>'Netherlands',
'AN'=>'Netherlands Antilles',
'NC'=>'New Caledonia',
'NZ'=>'New Zealand',
'NI'=>'Nicaragua',
'NE'=>'Niger',
'NG'=>'Nigeria',
'NU'=>'Niue',
'NF'=>'Norfolk Island',
'KP'=>'North Korea',
'MP'=>'Northern Mariana Islands',
'NO'=>'Norway',
'OM'=>'Oman',
'PK'=>'Pakistan',
'PW'=>'Palau',
'PA'=>'Panama',
'PG'=>'Papua New Guinea',
'PY'=>'Paraguay',
'PE'=>'Peru',
'PH'=>'Philippines',
'PN'=>'Pitcairn',
'PL'=>'Poland',
'PT'=>'Portugal',
'PR'=>'Puerto Rico',
'QA'=>'Qatar',
'RE'=>'Reunion',
'RO'=>'Romania',
'RU'=>'Russia',
'RW'=>'Rwanda',
'SH'=>'Saint Helena',
'KN'=>'Saint Kitts And Nevis',
'LC'=>'Saint Lucia',
'PM'=>'Saint Pierre And Miquelon',
'VC'=>'Saint Vincent And The Grenadines',
'SM'=>'San Marino',
'ST'=>'Sao Tome And Principe',
'SA'=>'Saudi Arabia',
'SN'=>'Senegal',
'SC'=>'Seychelles',
'SL'=>'Sierra Leone',
'SG'=>'Singapore',
'SK'=>'Slovak Republic',
'SI'=>'Slovenia',
'SB'=>'Solomon Islands',
'SO'=>'Somalia',
'ZA'=>'South Africa',
'GS'=>'South Georgia And South Sandwich Islands',
'KR'=>'South Korea',
'ES'=>'Spain',
'LK'=>'Sri Lanka',
'SD'=>'Sudan',
'SR'=>'Suriname',
'SJ'=>'Svalbard And Jan Mayen',
'SZ'=>'Swaziland',
'SE'=>'Sweden',
'CH'=>'Switzerland',
'SY'=>'Syria',
'TW'=>'Taiwan',
'TJ'=>'Tajikistan',
'TZ'=>'Tanzania',
'TH'=>'Thailand',
'TG'=>'Togo',
'TK'=>'Tokelau',
'TO'=>'Tonga',
'TT'=>'Trinidad And Tobago',
'TN'=>'Tunisia',
'TR'=>'Turkey',
'TM'=>'Turkmenistan',
'TC'=>'Turks And Caicos Islands',
'TV'=>'Tuvalu',
'UG'=>'Uganda',
'UA'=>'Ukraine',
'AE'=>'United Arab Emirates',
'UK'=>'United Kingdom',
'US'=>'United States',
'UM'=>'United States Minor Outlying Islands',
'UY'=>'Uruguay',
'UZ'=>'Uzbekistan',
'VU'=>'Vanuatu',
'VA'=>'Vatican City (Holy See)',
'VE'=>'Venezuela',
'VN'=>'Vietnam',
'VG'=>'Virgin Islands (British)',
'VI'=>'Virgin Islands (US)',
'WF'=>'Wallis And Futuna Islands',
'EH'=>'Western Sahara',
'WS'=>'Western Samoa',
'YE'=>'Yemen',
'YU'=>'Yugoslavia',
'ZM'=>'Zambia',
'ZW'=>'Zimbabwe'
);

return $countries;

}

