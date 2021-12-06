


// Symbols that must be the same in header-file and implementation mode:
//
#define STB_TEXTEDIT_CHARTYPE char        //            the character type
#define STB_TEXTEDIT_POSITIONTYPE u16        //    small type that is a valid cursor position
#define STB_TEXTEDIT_UNDOSTATECOUNT 64  //    the number of undo states to allow
#define STB_TEXTEDIT_UNDOCHARCOUNT 512   //    the number of characters to store in the undo buffer
//
// Symbols you must define for implementation mode:
//

struct textEditString
{
  char* string;
  int length;  
};

int textEditStrLen(textEditString* obj)
{
  return obj->length;
}

#define STB_TEXTEDIT_STRING               //          the type of object representing a string being edited,
//                                                       typically this is a wrapper object with other data you need
//
#define STB_TEXTEDIT_STRINGLEN(obj)       //          the length of the string (ideally O(1))
#define STB_TEXTEDIT_LAYOUTROW(&r,obj,n)  //          returns the results of laying out a line of characters
//                                                     starting from character #n (see discussion below)
#define STB_TEXTEDIT_GETWIDTH(obj,n,i)    //          returns the pixel delta from the xpos of the i'th character
//                                                       to the xpos of the i+1'th char for a line of characters
//                                                       starting at character #n (i.e. accounts for kerning
//                                                       with previous char)
#define STB_TEXTEDIT_KEYTOTEXT(k)         //          maps a keyboard input to an insertable character
//                                                    (return type is int, -1 means not valid to insert)
#define STB_TEXTEDIT_GETCHAR(obj,i)       //           returns the i'th character of obj, 0-based
#define STB_TEXTEDIT_NEWLINE              //          the character returned by _GETCHAR() we recognize

#define STB_TEXTEDIT_DELETECHARS(obj,i,n)    //       delete n characters starting at i
#define STB_TEXTEDIT_INSERTCHARS(obj,i,c*,n)  //      insert n characters at i (pointed to by STB_TEXTEDIT_CHARTYPE*)
//
#define STB_TEXTEDIT_K_SHIFT     //  a power of two that is or'd in to a keyboard input to represent the shift key
//
#define STB_TEXTEDIT_K_LEFT       // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT       //keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP          //keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN        //keyboard input to move cursor down
#define STB_TEXTEDIT_K_PGUP        //keyboard input to move cursor up a page
#define STB_TEXTEDIT_K_PGDOWN      //keyboard input to move cursor down a page
#define STB_TEXTEDIT_K_LINESTART   //keyboard input to move cursor to start of line  #define e.g. HOME
#define STB_TEXTEDIT_K_LINEEND     //keyboard input to move cursor to end of line    #define e.g. END
#define STB_TEXTEDIT_K_TEXTSTART   //keyboard input to move cursor to start of text  #define e.g. ctrl-HOME
#define STB_TEXTEDIT_K_TEXTEND     //keyboard input to move cursor to end of text    #define e.g. ctrl-END
#define STB_TEXTEDIT_K_DELETE      //keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE   //keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO        //keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO        //keyboard input to perform redo
//
//
#define STB_TEXTEDIT_IMPLEMENTATION
