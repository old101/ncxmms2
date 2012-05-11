/**
 *  This file is a part of ncxmms2, an XMMS2 Client.
 *
 *  Copyright (C) 2011-2012 Pavel Kunavin <tusk.kun@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <curses.h>

#include "window.h"
#include "window_p.h"
#include "size.h"
#include "rectangle.h"

using namespace ncxmms2;

Window::Window(const Rectangle& rect, Window *parent) :
    Object(parent),
    d(new WindowPrivate(rect.lines(), rect.cols(), rect.y(), rect.x(), parent))
{
    if (parent) {
        d->cursesWin = derwin(parent->d->cursesWin,
                              rect.lines(), rect.cols(), rect.y(), rect.x());
        parent->d->childrenWins.push_back(this);
    } else {
        d->cursesWin = newwin(rect.lines(), rect.cols(), rect.y(), rect.x());
    }
}

int Window::lines() const
{
    return d->lines;
}

int Window::cols() const
{
    return d->cols;
}

int Window::xPos() const
{
    return d->xPos;
}

int Window::yPos() const
{
    return d->yPos;
}

void Window::move(int x, int y)
{
    d->yPos = y;
    d->xPos = x;
    delwin(d->cursesWin);
    d->cursesWin = d->parent
                   ? derwin(d->parent->d->cursesWin, d->lines, d->cols, y, x)
                   : newwin(d->lines, d->cols, y, x);

    update();

    for (auto child : d->childrenWins)
        child->move(child->xPos(), child->yPos());
}

void Window::hide()
{
    d->isVisible = false;
    for (auto child : d->childrenWins)
        child->hide();
}

void Window::show()
{
    d->isVisible = true;
    paint(Rectangle(0, 0, cols(), lines()));
    for (auto child : d->childrenWins)
        child->show();
}

bool Window::isHidden() const
{
    return !d->isVisible;
}

void Window::setFocus()
{
    if (!d->parent)
        return;

    Window *old = d->parent->d->focusedWindow;
    d->parent->d->focusedWindow = this;
    if (old)
        old->update();
    update();
}

bool Window::hasFocus() const
{
    if (!d->parent)
        return true;

    return d->parent->d->focusedWindow == this;
}

void Window::keyPressedEvent(const KeyEvent& keyEvent)
{
    if (d->focusedWindow)
        d->focusedWindow->keyPressedEvent(keyEvent);
}

void Window::resize(const Size& size)
{
    d->lines = size.lines();
    d->cols = size.cols();

    delwin(d->cursesWin);
    d->cursesWin = d->parent
                   ? derwin(d->parent->d->cursesWin, d->lines, d->cols, d->yPos, d->xPos)
                   : newwin(d->lines, d->cols, d->yPos, d->xPos);

    update();
}

void Window::paint(const Rectangle& rect)
{

}

void Window::update()
{
    update(Rectangle(0, 0, cols(), lines()));
}

void Window::update(const Rectangle& rect)
{
    if (d->isVisible)
        paint(rect);
}

const std::string& Window::title() const
{
    return d->title;
}

void Window::setTitle(const std::string& title)
{
    d->title = title;
    titleChanged(title);
}

Window::~Window()
{
    delwin(d->cursesWin);
}

