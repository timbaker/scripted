/*
 * Copyright 2013, Tim Baker <treectrl@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "editnodevariabledialog.h"
#include "ui_editnodevariabledialog.h"

#include "node.h"
#include "project.h"
#include "projectdocument.h"
#include "scriptvariable.h"

EditNodeVariableDialog::EditNodeVariableDialog(ProjectDocument *doc, ScriptVariable *var, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditNodeVariableDialog)
{
    ui->setupUi(this);

    ui->name->setText(tr("\"%1\" (name is <i>%2</i>)<br>Type: %3")
                      .arg(var->label())
                      .arg(var->name())
                      .arg(var->type()));
    ui->value->setText(var->value());
    if (var->node() && var->node()->isEventNode())
        ui->value->setEnabled(false);

    if (var->variableRef().length()) {
        QString text;
        if (BaseNode *node = doc->project()->rootNode()->nodeByID(var->variableRefID())) {
            if (ScriptVariable *refVar = node->variable(var->variableRef())) {
                text = tr("Variable: \"%1\" (name is %2)")
                        .arg(refVar->label().toHtmlEscaped())
                        .arg(refVar->name().toHtmlEscaped());
                if (node->isProjectRootNode())
                    text += tr("<br>Source: current project");
                else
                    text += tr("<br>Source: node \"%1\" ID %2")
                            .arg(node->label().toHtmlEscaped())
                            .arg(node->id());
                text += tr("<br>Type: %1").arg(refVar->type());
                if (!var->acceptsType(refVar))
                    text += tr("<br><span style=\"color:#ff0000\">The type of the referenced variable is incompatible with this variable's type.</span>");
            } else {
                text = tr("Variable: name is %1")
                        .arg(var->variableRef().toHtmlEscaped());
                if (node->isProjectRootNode())
                    text += tr("<br>Source: current project");
                else
                    text += tr("<br>Source: node \"%1\" ID %2")
                            .arg(node->label().toHtmlEscaped())
                            .arg(node->id());
                text += tr("<br><span style=\"color:#ff0000\">The variable \"%1\" is unknown.</span>")
                        .arg(var->variableRef().toHtmlEscaped());
            }
        } else {
            text = tr("Variable: \"%1\"<br>Source: node ID %2")
                    .arg(var->variableRef().toHtmlEscaped())
                    .arg(var->variableRefID());
            text += tr("<br><span style=\"color:#ff0000\">The source node is invalid.</span>");
        }
        ui->reference->setText(text);
    } else
        ui->reference->setText(tr("&lt;none&gt;"));
}

EditNodeVariableDialog::~EditNodeVariableDialog()
{
    delete ui;
}

QString EditNodeVariableDialog::value() const
{
    return ui->value->text();
}
