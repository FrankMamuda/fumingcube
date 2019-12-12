/*
 * Copyright (C) 2019 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

/*
    1) we send a CIDRequest to get an exact structure match
       however it often fails (Sodium bicarbonate for example)
    2) we send a CIDRequestSimilar to get related compounds
       it often returns a long list of partial matches, and in
       many cases our desired compound is not the first item on
       the list
    3) in this case we open a StructureBrowser and offer the
       user to choose between multiple compounds, and to do that:
    4) we send a IUPACName request (to get a name) and
       FormulaRequestBrowser (to get a formula)
       user can now either select the first entry or go to the
       next one, and so on.
       (we can also pre-cache formulas in this step)
    5) when the required compound cid is found, we finally send
       a DataRequest and a FormulaRequest (if needed) and
       continue to extract properties as usual

    as of now the first four steps are not implemented properly
    or at all, so there's lots of work to be done
*/

/*
 * includes
 */
#include "structurebrowser.h"
#include "ui_structurebrowser.h"

StructureBrowser::StructureBrowser( QWidget *parent ) : QDialog( parent ), ui( new Ui::StructureBrowser ) {
    // get structure https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/7843/PNG
    // get name https://pubchem.ncbi.nlm.nih.gov/rest/pug/compound/cid/7843/property/IUPACName/TXT
    this->ui->setupUi(this);
}

/**
 * @brief StructureBrowser::~StructureBrowser
 */
StructureBrowser::~StructureBrowser() {
    delete this->ui;
}
