/* Copyright (C) 2011 MoSync AB

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License, version 2, as published by
 the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with this program; see the file COPYING.  If not, write to the Free
 Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 02111-1307, USA.
 */

/**
 * @file ListViewWidget.mm
 * @author Bogdan Iusco
 *
 * @brief ListView widget interface file.
 * A ListView widget is used to present an indexed/grouped list of items.
 */

/**
 * Default title for the delete-confirmation button, which appears on the right side of the cell.
 */
#define DEFAULT_DELETE_BUTTON_TITLE "Delete";

#include "Platform.h"
#include <helpers/cpp_defs.h>
#include <helpers/CPP_IX_WIDGET.h>

#import "ListViewWidget.h"
#import "ListViewSectionWidget.h"
#import "ListViewItemWidget.h"
#import "NSStringExpanded.h"

/**
 * Private methods for ListViewWidget.
 */
@interface ListViewWidget (hidden)

/**
 * Create a UITableView object and set the delegates.
 * If the list already has an view object it will be released and
 * a new one will be created.
 * The newest table view style will be set accordingly to the list view's type.
 * All its children will be realeased.
 */
-(void)createTableView;

/**
 * Set the list view type.
 * Setter for MAW_LIST_VIEW_TYPE.
 * All its children will be deleted.
 * Will create a new UITableView object.
 * @param listType Must be one of the following values:
 * - MAW_LIST_VIEW_TYPE_DEFAULT
 * - MAW_LIST_VIEW_TYPE_ALPHABETICAL
 * - MAW_LIST_VIEW_TYPE_SEGMENTED
 * @return An string object containing one of the following values:
 * - MAW_RES_OK if the list's type was set.
 * - MAW_RES_INVALID_PROPERTY_VALUE if the listType param was invalid.
 */
-(int)setListViewTypeProperty:(NSString*)listType;

/**
 * Get the list view type property.
 * Getter for MAW_LIST_VIEW_TYPE.
 * @return An NSString object containg the following valus:
 * - MAW_LIST_VIEW_TYPE_DEFAULT
 * - MAW_LIST_VIEW_TYPE_ALPHABETICAL
 * - MAW_LIST_VIEW_TYPE_SEGMENTED
 */
-(NSString*)getListViewTypeProperty;

@end

@implementation ListViewWidget

@synthesize type = _type;

/**
 * Init function.
 */
- (id)init
{
    self = [super init];
    if (self)
    {
        _type = ListViewTypeDefault;
        [self createTableView];
    }

    return self;
}

/**
 * Getter for the widget's view.
 */
-(UITableView*) tableView
{
    UITableView* tableView = (UITableView*) self.view;
    return tableView;
}

/**
 * Adds an widget to the end of the children list.
 * @param child Widget to be added.
 * @return One of the following constants:
 * - MAW_RES_OK if the child was added.
 * - MAW_RES_INVALID_LAYOUT if child's type is not as expected.
 * If list view's type is ListViewTypeDefault then the child's type must be
 * ListViewItemWidget, otherwise its type must be ListViewSectionWidget.
 */
- (int)addChild:(IWidget*)child
{
    if (_type == ListViewTypeDefault)
    {
      if ([child class] != [ListViewItemWidget class] &&
        [child superclass] != [ListViewItemWidget class])
      {
        return MAW_RES_INVALID_LAYOUT;
      }
      else
      {
          ListViewSectionWidget* section = (ListViewSectionWidget*)[_children objectAtIndex:0];
          [section addChild:child];
      }
    }
    else if (_type == ListViewTypeAlphabetical &&
             _type == ListViewTypeSegmented)
    {
        if ([child class] != [ListViewSectionWidget class] ||
            [child superclass] != [ListViewSectionWidget class])
        {
            return MAW_RES_INVALID_LAYOUT;
        }
        else
        {
            [self addChild:child];
            ListViewSectionWidget* section = (ListViewSectionWidget*) child;
            section.index = [_children count] - 1;
        }
    }
    return MAW_RES_OK;
}

/**
 * Insert a widget to a given index.
 * @param child Widget to be added.
 * @param index The index where the child should be inserted.
 * @return One of the following constants:
 * - MAW_RES_OK if the child was added.
 * - MAW_RES_INVALID_LAYOUT If list view's type is ListViewTypeDefault then the child's
 * type must be ListViewItemWidget, otherwise its type must be ListViewSectionWidget.
 * - MAW_RES_INVALID_INDEX if the index was out of bounds.
 */
- (int)insertChild:(IWidget*)child atIndex:(NSNumber*)index
{
    int result = MAW_RES_OK;
    if (_type == ListViewTypeDefault)
    {
        if ([child class] != [ListViewItemWidget class] &&
            [child superclass] != [ListViewItemWidget class])
        {
            result = MAW_RES_INVALID_LAYOUT;
        }
        else
        {
            ListViewSectionWidget* section = (ListViewSectionWidget*)[_children objectAtIndex:0];
            result = [section insertChild:child atIndex:index];
        }
    }
    else if (_type == ListViewTypeAlphabetical ||
             _type == ListViewTypeSegmented)
    {
        if ([child class] != [ListViewSectionWidget class] &&
            [child superclass] != [ListViewSectionWidget class])
        {
            result = MAW_RES_INVALID_LAYOUT;
        }
        else
        {
            result = [self insertChild:child atIndex:index toSubview:NO];
        }
    }
    return result;
}

/**
 * Remove a child from the widget.
 * The child's view will be removed its superview.
 * @param child Widget to remove.
 */
- (void)removeChild: (IWidget*)child
{
    [super removeChild:child fromSuperview:NO];
}

/**
 * Set a widget property value.
 * @param key Widget's property name that should be set.
 * @param value Widget's proeprty value that should be set.
 * @return One of the following values:
 * - MAW_RES_OK if the property was set.
 * - MAW_RES_INVALID_PROPERTY_NAME if the property name was invalid.
 * - MAW_RES_INVALID_PROPERTY_VALUE if the property value was invalid.
 */
- (int)setPropertyWithKey:(NSString*)key toValue:(NSString*)value
{
    int returnValue = MAW_RES_OK;

    if ([key isEqualToString:@MAW_LIST_VIEW_TYPE])
    {
        returnValue = [self setListViewTypeProperty:value];
    }
    else
    {
        returnValue = [super setPropertyWithKey:key toValue:value];
    }

    return returnValue;
}

/**
 * Get a widget property value.
 * @param key Widget's property name.
 * @return The property value, or nil if the property name is invalid.
 * The returned value should not be autoreleased. The caller will release the returned value.
 */
- (NSString*)getPropertyWithKey:(NSString*)key
{
    if ([key isEqualToString:@MAW_LIST_VIEW_TYPE])
    {
        return [[self getListViewTypeProperty] retain];
    }
    else
    {
        return [super getPropertyWithKey:key];
    }
}

#pragma mark UITableViewDataSource

/**
 * Return the number of rows in a given section of a table view.
 * @param tableView The table-view object requesting this information.
 * @param section An index number identifying a section in tableView.
 * @return The number of rows in section.
 */
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    ListViewSectionWidget* tableSection = [_children objectAtIndex:section];
    if (!tableSection)
    {
        return 0;
    }
    NSLog(@"IN %s tableSection countCells = %d", __FUNCTION__, [tableSection countCells]);
    return [tableSection countCells];
}

/**
 * Return the cell to insert in a particular location of the table view.
 * @param tableView The table-view object requesting this information.
 * @param indexPath An index path locating a row in tableView.
 * @return The cell to insert at the given row.
 */
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    INNativeUILog;
    UITableView* view = (UITableView*) self.view;
    UITableViewCell *cell = [view dequeueReusableCellWithIdentifier:@"Cell"];
    if (!cell)
    {
        ListViewSectionWidget* section = [_children objectAtIndex:indexPath.section];
        if (!section)
        {
            return nil;
        }

        ListViewItemWidget *cellWidget = [section cellWidgetAtIndex:indexPath.row];
        if (!cellWidget)
        {
            return nil;
        }
        cell = cellWidget.cell;
    }
    return cell;
}

/**
 * Return the number of sections in the table view.
 * @param tableView The table-view object requesting this information.
 * @return the number of sections in the table view.
 */
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return [_children count];
}

/**
 * Return the title of the header of the specified section of the table view.
 * @param tableView The table-view object requesting this information.
 * @param section An index number identifying a section of tableView.
 * @return A string to use as the title of the section header.
 */
- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    ListViewSectionWidget* tableViewSection = [_children objectAtIndex:section];
    if (!tableViewSection)
    {
        return nil;
    }

    return tableViewSection.headerTitle;
}

/**
 * Return the title of the footer of the specified section of the table view.
 * @param tableView The table-view object requesting this information.
 * @param section An index number identifying a section of tableView.
 * @return A string to use as the title of the section footer.
 */
- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
    ListViewSectionWidget* tableViewSection = [_children objectAtIndex:section];
    if (!tableViewSection)
    {
        return nil;
    }

    return tableViewSection.footerTitle;
}

/**
 * Verify that the given row is editable.
 * @param tableView The table-view object requesting this information.
 * @param indexPath An index path locating a row in tableView.
 * @return YES if the row indicated by indexPath is editable; otherwise, NO.
 */
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return YES;
}

/**
 * Verify if a given row can be moved to another location in the table view.
 * @param tableView The table-view object requesting this information.
 * @param indexPath An index path locating a row in tableView.
 * @return YES if the row can be moved; otherwise NO.
 */
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    return YES;
}

/**
 * Return the titles for the sections for a table view.
 * @param tableView The table-view object requesting this information.
 * @return An array of strings that serve as the title of sections in the table view and
 * appear in the index list on the right side of the table view.
 */
- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView
{
    NSMutableArray* array = [[[NSMutableArray alloc] init] autorelease];
    for (ListViewSectionWidget* section in _children)
    {
        if (section.title)
        {
            [array addObject:section.title];
        }
    }

    return array;
}

/**
 * Move a row at a specific location in the table view to another location.
 * @param tableView The table-view object requesting this action.
 * @param fromIndexPath An index path locating the row to be moved in tableView.
 * @param toIndexPath An index path locating the row in tableView that is the destination of the move.
 */
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
    NSLog(@"IN %s", __FUNCTION__);
}

#pragma mark UITableViewDelegate

/**
 * Return the height to use for a row in a specified location.
 * @param tableView The table-view object requesting this information.
 * @param indexPath An index path that locates a row in tableView.
 * @return A floating-point value that specifies the height (in points) that row should be.
 */
- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    ListViewSectionWidget* section = [_children objectAtIndex:indexPath.section];
    if (!section)
    {
        return 0;
    }
    ListViewItemWidget* cellWidget = [section cellWidgetAtIndex:indexPath.row];
    return cellWidget.height;
}

/**
 * A specified row is about to be selected.
 * @param tableView A table-view object informing the delegate about the impending selection.
 * @param indexPath An index path locating the row in tableView.
 * @return An index-path object that confirms or alters the selected row.
 */
- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	MAEvent event;
	event.type = EVENT_TYPE_WIDGET;
	MAWidgetEventData *eventData = new MAWidgetEventData;
	eventData->eventType = MAW_EVENT_ITEM_WILL_SELECT;
	eventData->widgetHandle = self.handle;
	eventData->listItemIndex = [indexPath row];
	event.data = (int)eventData;
	Base::gEventQueue.put(event);

    return indexPath;
}

/**
 * Called when a specified row is now selected.
 * @param tableView A table-view object informing the delegate about the new row selection.
 * @param indexPath An index path locating the row in tableView.
 */
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	MAEvent event;
	event.type = EVENT_TYPE_WIDGET;
	MAWidgetEventData *eventData = new MAWidgetEventData;
	eventData->eventType = MAW_EVENT_ITEM_CLICKED;
	eventData->widgetHandle = self.handle;
	eventData->listItemIndex = [indexPath row];
	event.data = (int)eventData;
	Base::gEventQueue.put(event);
}

/**
 * Changes the default title of the delete-confirmation button.
 * @param tableView The table-view object requesting this information.
 * @param indexPath An index-path object locating the row in its section.
 * @return A localized string to used as the title of the delete-confirmation button.
 */
- (NSString *)tableView:(UITableView *)tableView titleForDeleteConfirmationButtonForRowAtIndexPath:(NSIndexPath *)indexPath
{
    ListViewSectionWidget* section = [_children objectAtIndex:indexPath.section];
    if (!section)
    {
        return @DEFAULT_DELETE_BUTTON_TITLE;
    }
    ListViewItemWidget* cellWidget = [section cellWidgetAtIndex:indexPath.row];
    return cellWidget.deleteButtonTitle;
}

/**
 * Get the editing style of a row at a particular location in a table view.
 * @param tableView The table-view object requesting this information.
 * @param indexPath An index path locating a row in tableView.
 * @return The editing style of the cell for the row identified by indexPath.
 */
- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath
{
    ListViewSectionWidget* section = [_children objectAtIndex:indexPath.section];
    if (!section)
    {
        return UITableViewCellEditingStyleNone;
    }
    ListViewItemWidget* cellWidget = [section cellWidgetAtIndex:indexPath.row];
    if (!cellWidget)
    {
        return UITableViewCellEditingStyleNone;
    }
    return cellWidget.editingStyle;
}

#pragma mark ListViewDelegate

/**
 * Delete an list view item index from the list.
 * @param indexPath List item index to be deleted.
 */
-(void)deleteItemAtIndexPath:(NSIndexPath*) indexPath
{
    INNativeUILog;
    [self.tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath]
                          withRowAnimation:UITableViewRowAnimationAutomatic];
}

/**
 * Insert an list view item into the list.
 * @param indexPath List item index where to be inserted.
 */
-(void)insertItemAtIndexPath:(NSIndexPath*) indexPath
{
    [self.tableView insertRowsAtIndexPaths:[NSArray arrayWithObject:indexPath]
                          withRowAnimation:UITableViewRowAnimationAutomatic];
}

/**
 * Dealloc method.
 */
-(void) dealloc
{
    [super dealloc];
}
@end

/**
 * Private methods for ListViewWidget.
 */
@implementation ListViewWidget (hidden)

/**
 * Create a UITableView object and set the delegates.
 * If the list already has an view object it will be released and
 * a new one will be created.
 * The newest table view style will be set accordingly to the list view's type.
 * All its children will be realeased.
 */
-(void)createTableView
{
    [_children removeAllObjects];
    UITableView* tableView = (UITableView*)self.view;
    CGRect rect = CGRectZero;
    UIView* superview = nil;
    if (tableView)
    {
        rect = tableView.frame;
        tableView.delegate = nil;
        tableView.dataSource = nil;
        [tableView removeFromSuperview];
        superview = tableView.superview;
    }

    UITableViewStyle style = UITableViewStylePlain;
    if (_type == ListViewTypeSegmented)
    {
        style = UITableViewStyleGrouped;
    }
    tableView = [[UITableView alloc] initWithFrame:rect style:style];
    tableView.delegate = self;
    tableView.dataSource = self;
    self.view = tableView;

    if (superview)
    {
        [superview addSubview:tableView];
    }

    if (_type == ListViewTypeDefault)
    {
        ListViewSectionWidget* section = [[ListViewSectionWidget alloc] init];
        [self addChild:section toSubview:YES];
        section.index = 0;
        section.delegate = self;
    }
    [tableView release];
    tableView = nil;
}

/**
 * Set the list view type.
 * Setter for MAW_LIST_VIEW_TYPE.
 * All its children will be deleted.
 * Will create a new UITableView object.
 * @param listType Must be one of the following values:
 * - MAW_LIST_VIEW_TYPE_DEFAULT
 * - MAW_LIST_VIEW_TYPE_ALPHABETICAL
 * - MAW_LIST_VIEW_TYPE_SEGMENTED
 * @return An string object containing one of the following values:
 * - MAW_RES_OK if the list's type was set.
 * - MAW_RES_INVALID_PROPERTY_VALUE if the listType param was invalid.
 */
-(int)setListViewTypeProperty:(NSString*)listType
{
    if (![listType canParseNumber])
    {
        return MAW_RES_INVALID_PROPERTY_VALUE;
    }

    int returnValue = MAW_RES_OK;
    ListViewType previousType = _type;
    switch ([listType intValue])
    {
        case MAW_LIST_VIEW_TYPE_DEFAULT:
            _type = ListViewTypeDefault;
            break;
        case MAW_LIST_VIEW_TYPE_ALPHABETICAL:
            _type = ListViewTypeAlphabetical;
            break;
        case MAW_LIST_VIEW_TYPE_SEGMENTED:
            _type = ListViewTypeSegmented;
            break;
        default:
            returnValue = MAW_RES_INVALID_PROPERTY_VALUE;
    }

    if (previousType != _type)
    {
        [self createTableView];
    }

    return returnValue;
}

/**
 * Get the list view type property.
 * Getter for MAW_LIST_VIEW_TYPE.
 * @return An NSString object containg the following valus:
 * - MAW_LIST_VIEW_TYPE_DEFAULT
 * - MAW_LIST_VIEW_TYPE_ALPHABETICAL
 * - MAW_LIST_VIEW_TYPE_SEGMENTED
 */
-(NSString*)getListViewTypeProperty
{
    int listType = MAW_LIST_VIEW_TYPE_DEFAULT;
    switch (_type)
    {
        case ListViewTypeAlphabetical:
            listType = MAW_LIST_VIEW_TYPE_ALPHABETICAL;
            break;
        case ListViewTypeSegmented:
            listType = MAW_LIST_VIEW_TYPE_SEGMENTED;
            break;
        case ListViewTypeDefault:
        default:
            listType = MAW_LIST_VIEW_TYPE_DEFAULT;
            break;
    }
    return [NSString stringWithFormat:@"%d", listType];
}

@end
