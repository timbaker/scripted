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

#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

class LineNumberArea;

struct ParenthesisInfo
{
    char character;
    int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData();

    QVector<ParenthesisInfo *> parentheses();
    void insert(ParenthesisInfo *info);

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class Highlighter : public QSyntaxHighlighter
{
public:
    Highlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);
    void findMatches(const QString &text, TextBlockData *data, char ch1, char ch2);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

class LuaEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    LuaEditor();

private slots:
    void cursorPositionChanged();

    void matchParentheses();

    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

protected:
    void matchParentheses(char ch1, char ch2);
    bool matchLeftParenthesis(char ch1, char ch2, QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(char ch1, char ch2, QTextBlock currentBlock, int index, int numLeftParentheses);
    void createParenthesisSelection(int pos);

    void resizeEvent(QResizeEvent *e);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    LineNumberArea *lineNumberArea;
    QColor mCurrentLineColor;

    friend class LineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(LuaEditor *editor) : QWidget(editor)
    {
        codeEditor = editor;
    }

    QSize sizeHint() const
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    LuaEditor *codeEditor;
};

#endif // LUAEDITOR_H
