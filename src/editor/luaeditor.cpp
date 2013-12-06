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

#include "luaeditor.h"

#include "editor_global.h"

#include <QApplication>
#include <QPainter>

TextBlockData::TextBlockData()
{
    // Nothing to do
}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}

void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() &&
        info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}

/////

Highlighter::Highlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\band\\b" << "\\bbreak\\b" << "\\bdo\\b"
                    << "\\belse\\b" << "\\belseif\\b" << "\\bend\\b"
                    << "\\bfalse\\b" << "\\bfor\\b" << "\\bfunction\\b"
                    << "\\bgoto\\b" << "\\bif\\b" << "\\bin\\b"
                    << "\\blocal\\b" << "\\bnil\\b" << "\\bnot\\b"
                    << "\\bor\\b" << "\\brepeat\\b" << "\\breturn\\b"
                    << "\\bthen\\b" << "\\btrue\\b" << "\\buntil\\b"
                    << "\\bwhile\\b" << "\\brequire\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Numbers
    QTextCharFormat numbers;
    numbers.setForeground(Qt::magenta);
    rule.pattern = QRegExp("[0-9]");
    rule.format = numbers;
    highlightingRules.append(rule);

    // Single-line comment
    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // "String"
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.pattern = QRegExp( "(?:^|[^\\\\'])(\"(?:\\\\\"|\\\\(?!\")|[^\\\\\"^ä^ö^ü])*\")" );
    rule.pattern.setMinimal(true);
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 'String'
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\'.*\'");
    rule.pattern = QRegExp( "(?:^|[^\\\\\"])(\'(?:\\\\\'|\\\\(?!\')|[^\\\\\'])*\')" );
    rule.pattern.setMinimal(true);
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Function name()
    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(Qt::darkCyan);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // Multi-line comment --[[ ]]
    commentStartExpression = QRegExp("--\\[\\["); // --[[
    commentEndExpression = QRegExp("\\]\\]"); // ]]
    multiLineCommentFormat.setForeground(Qt::red);

}

void Highlighter::highlightBlock(const QString &text)
{
    TextBlockData *data = new TextBlockData;
    findMatches(text, data, '(', ')');
    findMatches(text, data, '{', '}');
    setCurrentBlockUserData(data);

    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
//return; ///////////////
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

void Highlighter::findMatches(const QString &text, TextBlockData *data, char ch1, char ch2)
{
    int leftPos = text.indexOf(ch1);
    while (leftPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ch1;
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf(ch1, leftPos + 1);
    }

    int rightPos = text.indexOf(ch2);
    while (rightPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ch2;
        info->position = rightPos;

        data->insert(info);

        rightPos = text.indexOf(ch2, rightPos + 1);
    }
}

////

LuaEditor::LuaEditor()
{
#if 1
    mCurrentLineColor = QColor(128, 255, 255, 32);
#else
    const QPalette palette = QApplication::palette();
    const QColor &fg = palette.color(QPalette::Highlight);
    const QColor &bg = palette.color(QPalette::Base);

    qreal smallRatio = .3;
    qreal largeRatio = .6;

    const qreal ratio = ((palette.color(QPalette::Text).value() < 128)
                         ^ (palette.color(QPalette::HighlightedText).value() < 128)) ? smallRatio : largeRatio;

    const QColor &col = QColor::fromRgbF(fg.redF() * ratio + bg.redF() * (1 - ratio),
                                         fg.greenF() * ratio + bg.greenF() * (1 - ratio),
                                         fg.blueF() * ratio + bg.blueF() * (1 - ratio));
    mCurrentLineColor = col;
#endif

    connect(this, SIGNAL(cursorPositionChanged()), SLOT(cursorPositionChanged()));

    lineNumberArea = new LineNumberArea(this);
    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    mSyntaxTimer.setInterval(750);
    mSyntaxTimer.setSingleShot(true);
    connect(&mSyntaxTimer, SIGNAL(timeout()), SLOT(checkSyntax()));

    connect(this, SIGNAL(textChanged()), &mSyntaxTimer, SLOT(start()));
}

void LuaEditor::cursorPositionChanged()
{
    QList<QTextEdit::ExtraSelection> selections;
    setExtraSelections(selections);

    highlightCurrentLine();
    matchParentheses();
}

void LuaEditor::matchParentheses()
{
    matchParentheses('(', ')');
    matchParentheses('{', '}');
}

void LuaEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void LuaEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = mCurrentLineColor; //QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);
}

void LuaEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

#include "luautils.h"

void LuaEditor::checkSyntax()
{
    LuaState L;
    L.loadString(toPlainText(), QLatin1String("chunk"));
    QString error = L.errorString();
    if (!error.isEmpty()) {
        int n = error.indexOf(QLatin1String("]:"));
        if (n >= 0)
            error = error.mid(n + 2);
    }
    emit syntaxError(error);
}

void LuaEditor::matchParentheses(char ch1, char ch2)
{
    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data) {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i) {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == ch1) {
                if (matchLeftParenthesis(ch1, ch2, textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            } else if (info->position == curPos - 1 && info->character == ch2) {
                if (matchRightParenthesis(ch1, ch2, textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool LuaEditor::matchLeftParenthesis(char ch1, char ch2, QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i) {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == ch1) {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == ch2 && numLeftParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numLeftParentheses;
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(ch1, ch2, currentBlock, 0, numLeftParentheses);

    return false;
}

bool LuaEditor::matchRightParenthesis(char ch1, char ch2, QTextBlock currentBlock, int i, int numRightParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i) {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == ch2) {
            ++numRightParentheses;
            continue;
        }
        if (info->character == ch1 && numRightParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numRightParentheses;
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid()) {
#if 1 // BUG IN ORIGINAL
        data = static_cast<TextBlockData *>(currentBlock.userData());
        parentheses = data->parentheses();
        return matchRightParenthesis(ch1, ch2, currentBlock, parentheses.size() - 1, numRightParentheses);
#else
        return matchRightParenthesis(ch1, ch2, currentBlock, 0, numRightParentheses);
#endif
    }

    return false;
}

void LuaEditor::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setForeground(Qt::red);
    format.setBackground(QColor(0xb4, 0xee, 0xb4));
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}

void LuaEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void LuaEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(232, 232, 232));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    painter.setPen(QColor(164, 164, 164));
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen((blockNumber == textCursor().blockNumber()) ?
                               QColor(96, 96, 96) : QColor(164, 164, 164));
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

int LuaEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}
