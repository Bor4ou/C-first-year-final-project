#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <windows.h>
#include <commctrl.h>

// Define brush for customizing GUI coloring
HBRUSH brush = NULL;

// Textboxes for inputs
HWND textboxLeadDr, textboxPartC, textboxDis, textboxSDate, textboxEDate, textboxDrName, textboxSpec, textboxtrialID, textboxdoctorID;

// Labels referring texboxes
HWND lblLeadDr, lblPartC, lblDis, lblSDate, lblEDate, lblDrName, lblSpec, lbltrialID, lbldoctorID, lblhint1, lblhint2;

// Buttons for executing the insert operation
HWND btnInsert, btnEdit;

// CRUD Operations menus
HMENU menuBar, insertMenu, viewMenu;

int selectedOption = -1; // No option selected yet

// Enumerate the textboxes and buttons that feed data to the DB
enum TextBoxes_ID
{
    // For "Trials" Table
    TextBox_LeadDr,
    TextBox_ParticipantCount,
    TextBox_Disease,
    TextBox_StartDate,
    TextBox_EndDate,
    // For "Doctors" table
    TextBox_DrName,
    TextBox_Specialty,
    // IDs
    TextBox_TrialID,
    TextBox_DoctorID
};

enum Buttons_ID
{
    Button_Insert = 10,
    Button_Edit = 11
};

// Suboptions for Insert New...
enum Insert_Options
{
    Insert_Into_Trials = 20,
    Insert_Into_Doctors = 21
};

// Suboptions for View
enum EditView_Options
{
    EditView_Trials = 40,
    EditView_Doctors = 41
};

// Function prototypes
HWND CreateTextBox(int x, int y, int height, int width, HWND hwnd, HINSTANCE hInstn, int textboxID);
HWND CreateLabel(const char* lblTitle, int x, int y, int height, int width, HWND hwnd, HINSTANCE hInst);
HWND CreateButton(const char* btnTitle, int x, int y, int height, int width, HWND hwnd, HINSTANCE hInst, int buttonID);
HWND ViewTrials(HWND hwnd, int colwidth);
HWND ViewDoctors(HWND hwnd, int colwidth);
void SetFont(HWND element, int fontSize, int fontWeight, const char *name);
void CreateMenuBar(HWND hwnd);
void HideElements(void);
bool IsNum(const char *str);
bool OnlyAlphaChars(const char *str);
bool IsLeapYear(int year);
bool ValidateDate(const char *date);

// Handles the messages (commands) which represent 
// the ongoing events by communicating with the API
// (e.g. button clicks and etc.)
LRESULT CALLBACK WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wPar, LPARAM lPar)
{
    switch (msg)
    {
        // Make background color for labels transparent
        case WM_CTLCOLORSTATIC:
        {
            HDC labelBGColor = (HDC)wPar;
            SetBkMode(labelBGColor, TRANSPARENT);
            // return NULL brush to make transparency
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        // Black divider line
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC hdc = BeginPaint(hwnd, &paint);

            // Pen color
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(hdc, pen);

            // Draw the line vertically
            MoveToEx(hdc, 375, 0, NULL);
            LineTo(hdc, 375, 340);

            // Remove pen and end the drawing
            SelectObject(hdc, oldPen);
            DeleteObject(pen);

            EndPaint(hwnd, &paint);
        }
        break;
        // Quit program
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0; 
        case WM_COMMAND:
            switch (LOWORD(wPar))
            {
                // Show the necessary elements depending on option selected
                // Insert trial
                case Insert_Into_Trials:
                    selectedOption = Insert_Into_Trials;

                    ShowWindow(lblLeadDr, SW_SHOW);
                    ShowWindow(lblPartC, SW_SHOW);
                    ShowWindow(lblDis, SW_SHOW);
                    ShowWindow(lblSDate, SW_SHOW);
                    ShowWindow(lblEDate, SW_SHOW);
                    ShowWindow(textboxLeadDr, SW_SHOW);
                    ShowWindow(textboxPartC, SW_SHOW);
                    ShowWindow(textboxDis, SW_SHOW);
                    ShowWindow(textboxSDate, SW_SHOW);
                    ShowWindow(textboxEDate, SW_SHOW);

                    ShowWindow(lblDrName, SW_HIDE);
                    ShowWindow(lblSpec, SW_HIDE);
                    ShowWindow(textboxDrName, SW_HIDE);
                    ShowWindow(textboxSpec,SW_HIDE);

                    ShowWindow(btnInsert, SW_SHOW);
                    ShowWindow(btnEdit, SW_HIDE);

                    ShowWindow(lbltrialID, SW_HIDE);
                    ShowWindow(textboxtrialID, SW_HIDE);
                    ShowWindow(lbldoctorID, SW_HIDE);
                    ShowWindow(textboxdoctorID, SW_HIDE);

                    ShowWindow(lblhint1, SW_HIDE);
                break;
                // Insert Doctor
                case Insert_Into_Doctors:
                    selectedOption = Insert_Into_Doctors;

                    ShowWindow(lblLeadDr, SW_HIDE);
                    ShowWindow(lblPartC, SW_HIDE);
                    ShowWindow(lblDis, SW_HIDE);
                    ShowWindow(lblSDate, SW_HIDE);
                    ShowWindow(lblEDate, SW_HIDE);
                    ShowWindow(textboxLeadDr, SW_HIDE);
                    ShowWindow(textboxPartC, SW_HIDE);
                    ShowWindow(textboxDis, SW_HIDE);
                    ShowWindow(textboxSDate, SW_HIDE);
                    ShowWindow(textboxEDate, SW_HIDE);

                    ShowWindow(lblDrName, SW_SHOW);
                    ShowWindow(lblSpec, SW_SHOW);
                    ShowWindow(textboxDrName, SW_SHOW);
                    ShowWindow(textboxSpec,SW_SHOW);

                    ShowWindow(btnInsert, SW_SHOW);
                    ShowWindow(btnEdit, SW_HIDE);

                    ShowWindow(lbltrialID, SW_HIDE);
                    ShowWindow(textboxtrialID, SW_HIDE);
                    ShowWindow(lbldoctorID, SW_HIDE);
                    ShowWindow(textboxdoctorID, SW_HIDE);

                    ShowWindow(lblhint1, SW_HIDE);
                break;
                // Handle data insertion into table
                case Button_Insert:
                    // Trial insert
                    if (selectedOption == Insert_Into_Trials)
                    {
                        char leadDr[100] = "", partCount[10] = "", disease[100] = "", SDate[11] = "", EDate[11] = "";

                        GetWindowText(textboxLeadDr, leadDr, sizeof(leadDr));
                        GetWindowText(textboxPartC, partCount, sizeof(partCount));
                        GetWindowText(textboxDis, disease, sizeof(disease));
                        GetWindowText(textboxSDate, SDate, sizeof(SDate));
                        GetWindowText(textboxEDate, EDate, sizeof(EDate));
                        char year1[5] = {SDate[6], SDate[7], SDate[8], SDate[9], '\0'};
                        char year2[5] = {EDate[6], EDate[7], EDate[8], EDate[9], '\0'};
                        char month1[3] = {SDate[3], SDate[4], '\0'};
                        char month2[3] = {EDate[3], EDate[4], '\0'};
                        char day1[3] = {SDate[0], SDate[1], '\0'};
                        char day2[3] = {EDate[0], EDate[1], '\0'};
                        int startDateInt = atoi(year1) * 10000 + atoi(month1) * 100 + atoi(day1);
                        int endDateInt = atoi(year2) * 10000 + atoi(month2) * 100 + atoi(day2);
                        // Validations of input
                        if (strlen(leadDr) == 0 || strlen(partCount) == 0 || strlen(disease) == 0 || strlen(SDate) == 0 || strlen(EDate) == 0)
                        {
                            MessageBox(hwnd, "Fill in all fields!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!OnlyAlphaChars(leadDr) || !OnlyAlphaChars(disease))
                        {
                            MessageBox(hwnd, "Mistake in either doctor or disease name!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (strlen(partCount) > 6 || !IsNum(partCount) || atoi(partCount) <= 0)
                        {
                            MessageBox(hwnd, "Enter valid participant count!\n(6 digit limit, numeric only, greater than 0)", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!ValidateDate(SDate) || !ValidateDate(EDate) || startDateInt > endDateInt)
                        {
                            MessageBox(hwnd, "Check dates!\nValid date format: DD/MM/YYYY", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else
                        {
                            // Open db and statement handlers
                            sqlite3* db;
                            sqlite3_stmt* stmt;
                            int dbstate = sqlite3_open("Trials.db", &db);
                            if (dbstate != SQLITE_OK)
                            {
                                MessageBox(hwnd, "Could not open database!", "Error", MB_OK | MB_ICONERROR);
                                break;
                            }

                            // Check if the doctor exists in the Doctors table
                            const char* checkDoctorQuery = "SELECT DoctorID FROM Doctors WHERE DoctorName = ?";
                            sqlite3_prepare_v2(db, checkDoctorQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                            dbstate = sqlite3_step(stmt);

                            int doctorID = -1;  // Default value if doctor does not exist
                            if (dbstate == SQLITE_ROW)
                            {
                                // Doctor exists so get the ID
                                doctorID = sqlite3_column_int(stmt, 0);
                            }
                            else if (dbstate == SQLITE_DONE)
                            {
                                // Doctor doesn't exist so insert the doctor into Doctors
                                const char* insertDoctorQuery = "INSERT INTO Doctors (DoctorName) VALUES (?)";
                                sqlite3_prepare_v2(db, insertDoctorQuery, -1, &stmt, NULL);
                                sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                                sqlite3_step(stmt);
                                sqlite3_reset(stmt);
                                sqlite3_clear_bindings(stmt);  // Execute inserting of doctor

                                // Get the DoctorID of the inserted doctor
                                const char* getDoctorIDQuery = "SELECT DoctorID FROM Doctors WHERE DoctorName = ?";
                                sqlite3_prepare_v2(db, getDoctorIDQuery, -1, &stmt, NULL);
                                sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                                dbstate = sqlite3_step(stmt); 
                                sqlite3_reset(stmt);
                                sqlite3_clear_bindings(stmt); // Execute getting the doctor's ID
                                if (dbstate == SQLITE_ROW)
                                {
                                    doctorID = sqlite3_column_int(stmt, 0);
                                }
                            }
                            sqlite3_finalize(stmt);

                            if (doctorID == -1)
                            {
                                // If DoctorID is still -1, something went wrong
                                MessageBox(hwnd, "Failed to insert or fetch Doctor ID!", "Error", MB_OK | MB_ICONERROR);
                                sqlite3_close(db);
                                break;
                            }

                            // Check if the trial with the same details exists
                            const char* checkTrialQuery = "SELECT COUNT(*) FROM Trials WHERE LeadDoctor = ? AND Disease = ? AND StartDate = ? AND EndDate = ?";
                            sqlite3_prepare_v2(db, checkTrialQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 2, disease, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 3, SDate, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 4, EDate, -1, SQLITE_STATIC);
                            sqlite3_step(stmt);
                            int found = sqlite3_column_int(stmt, 0);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);
                            sqlite3_finalize(stmt);

                            // If the trial already exists, show an error and stop
                            if (found > 0)
                            {
                                // Exit the function and don't insert trial
                                MessageBox(hwnd, "This trial already exists in the database!", "Error", MB_OK | MB_ICONERROR);
                                sqlite3_close(db);
                                break;
                            }

                            // Insert the trial data
                            const char* insertTrialQuery = "INSERT INTO Trials (LeadDoctor, ParticipantCount, Disease, StartDate, EndDate) VALUES (?, ?, ?, ?, ?)";
                            sqlite3_prepare_v2(db, insertTrialQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 2, atoi(partCount));
                            sqlite3_bind_text(stmt, 3, disease, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 4, SDate, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 5, EDate, -1, SQLITE_STATIC);
                            sqlite3_step(stmt);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);

                            // End insertion and quit DB
                            sqlite3_finalize(stmt);
                            sqlite3_close(db);

                            MessageBox(hwnd, "Trial data inserted successfully!", "Notice", MB_OK | MB_ICONINFORMATION);
                            SetWindowText(textboxLeadDr, "");
                            SetWindowText(textboxDis, "");
                            SetWindowText(textboxPartC, "");
                            SetWindowText(textboxSDate, "");
                            SetWindowText(textboxEDate, "");
                        }
                    }
                    // Doctor insert
                    else if (selectedOption == Insert_Into_Doctors)
                    {
                        char drName[100] = "", specialty[100] = "";

                        GetWindowText(textboxDrName, drName, sizeof(drName));
                        GetWindowText(textboxSpec, specialty, sizeof(specialty));
                        // Validations of input
                        if (strlen(drName) == 0 || strlen(specialty) == 0)
                        {
                            MessageBox(hwnd, "Fill in all fields!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!OnlyAlphaChars(drName) || !OnlyAlphaChars(specialty))
                        {
                            MessageBox(hwnd, "Mistake in either doctor or specialty name", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else
                        {
                            // Open db and statement handlers
                            sqlite3* db;
                            sqlite3_stmt* stmt;
                            int dbstate = sqlite3_open("Trials.db", &db);
                            if (dbstate != SQLITE_OK)
                            {
                                MessageBox(hwnd, "Could not open database!", "Error", MB_OK | MB_ICONERROR);
                            }

                            // Check if a doctor with the same details exists
                            const char* checkDoctorQuery = "SELECT COUNT(*) FROM Doctors WHERE DoctorName = ?";
                            sqlite3_prepare_v2(db, checkDoctorQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, drName, -1, SQLITE_STATIC);
                            sqlite3_step(stmt);
                            int found = sqlite3_column_int(stmt, 0);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);
                            sqlite3_finalize(stmt);
                            if (found > 0)
                            {
                                // Exit the function and don't insert doctor
                                MessageBox(hwnd, "This doctor already exists in the database!", "Error", MB_OK | MB_ICONERROR);
                                sqlite3_close(db);
                                break;
                            }

                            const char* insertDoctorQuery = "INSERT INTO Doctors (DoctorName, Specialty) VALUES (?, ?)";
                            sqlite3_prepare_v2(db, insertDoctorQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, drName, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 2, specialty, -1, SQLITE_STATIC);
                            sqlite3_step(stmt);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);

                            // End insertion and quit DB
                            sqlite3_finalize(stmt);
                            sqlite3_close(db);

                            MessageBox(hwnd, "Doctor data inserted successfully!", "Notice", MB_OK | MB_ICONINFORMATION);
                            SetWindowText(textboxDrName, "");
                            SetWindowText(textboxSpec, "");
                        }
                    }
                break;
                // Handle viewing the data from the tables and the edit fields
                case EditView_Trials:
                    selectedOption = EditView_Trials;

                    SetWindowPos(hwnd, NULL, CW_USEDEFAULT, CW_USEDEFAULT, 950, 400, SWP_NOMOVE | SWP_NOZORDER);

                    ShowWindow(lbltrialID, SW_SHOW);
                    ShowWindow(textboxtrialID, SW_SHOW);
                    ShowWindow(lbldoctorID, SW_HIDE);
                    ShowWindow(textboxdoctorID, SW_HIDE);

                    ShowWindow(lblLeadDr, SW_SHOW);
                    ShowWindow(lblPartC, SW_SHOW);
                    ShowWindow(lblDis, SW_SHOW);
                    ShowWindow(lblSDate, SW_SHOW);
                    ShowWindow(lblEDate, SW_SHOW);
                    ShowWindow(textboxLeadDr, SW_SHOW);
                    ShowWindow(textboxPartC, SW_SHOW);
                    ShowWindow(textboxDis, SW_SHOW);
                    ShowWindow(textboxSDate, SW_SHOW);
                    ShowWindow(textboxEDate, SW_SHOW);

                    ShowWindow(lblDrName, SW_HIDE);
                    ShowWindow(lblSpec, SW_HIDE);
                    ShowWindow(textboxDrName, SW_HIDE);
                    ShowWindow(textboxSpec, SW_HIDE);
                    
                    ShowWindow(btnEdit, SW_SHOW);

                    ShowWindow(lblhint1, SW_HIDE);
                    ShowWindow(lblhint2, SW_HIDE);
                    // Remove existing view (if there is)
                    HWND existingDoctorsView = FindWindowEx(hwnd, NULL, WC_LISTVIEW, NULL);
                    if (existingDoctorsView)
                    {
                        DestroyWindow(existingDoctorsView);
                    }

                    // Create Trials view
                    ViewTrials(hwnd, 100);
                break;
                case EditView_Doctors:
                    selectedOption = EditView_Doctors;
                    
                    SetWindowPos(hwnd, NULL, CW_USEDEFAULT, CW_USEDEFAULT, 665, 400, SWP_NOMOVE | SWP_NOZORDER);

                    ShowWindow(lbldoctorID, SW_SHOW);
                    ShowWindow(textboxdoctorID, SW_SHOW);
                    ShowWindow(lbltrialID, SW_HIDE);
                    ShowWindow(textboxtrialID, SW_HIDE);

                    ShowWindow(lblLeadDr, SW_HIDE);
                    ShowWindow(lblPartC, SW_HIDE);
                    ShowWindow(lblDis, SW_HIDE);
                    ShowWindow(lblSDate, SW_HIDE);
                    ShowWindow(lblEDate, SW_HIDE);
                    ShowWindow(textboxLeadDr, SW_HIDE);
                    ShowWindow(textboxPartC, SW_HIDE);
                    ShowWindow(textboxDis, SW_HIDE);
                    ShowWindow(textboxSDate, SW_HIDE);
                    ShowWindow(textboxEDate, SW_HIDE);

                    ShowWindow(lblDrName, SW_SHOW);
                    ShowWindow(lblSpec, SW_SHOW);
                    ShowWindow(textboxDrName, SW_SHOW);
                    ShowWindow(textboxSpec,SW_SHOW);

                    ShowWindow(btnEdit, SW_SHOW);

                    ShowWindow(lblhint1, SW_HIDE);
                    ShowWindow(lblhint2, SW_HIDE);
                    // Remove existing view (if there is)
                    HWND existingTrialsView = FindWindowEx(hwnd, NULL, WC_LISTVIEW, NULL);
                    if (existingTrialsView)
                    {
                        DestroyWindow(existingTrialsView);
                    }

                    // Create Doctors view
                    ViewDoctors(hwnd,  100);
                break;
                // Handle editing the data in the db 
                case Button_Edit:
                    if (selectedOption == EditView_Trials)
                    {
                        char trialID[10] = "", leadDr[100] = "", partCount[10] = "", disease[100] = "", SDate[11] = "", EDate[11] = "";
                        
                        GetWindowText(textboxtrialID, trialID, sizeof(trialID));
                        GetWindowText(textboxLeadDr, leadDr, sizeof(leadDr));
                        GetWindowText(textboxPartC, partCount, sizeof(partCount));
                        GetWindowText(textboxDis, disease, sizeof(disease));
                        GetWindowText(textboxSDate, SDate, sizeof(SDate));
                        GetWindowText(textboxEDate, EDate, sizeof(EDate));
                        char year1[5] = {SDate[6], SDate[7], SDate[8], SDate[9], '\0'};
                        char year2[5] = {EDate[6], EDate[7], EDate[8], EDate[9], '\0'};
                        char month1[3] = {SDate[3], SDate[4], '\0'};
                        char month2[3] = {EDate[3], EDate[4], '\0'};
                        char day1[3] = {SDate[0], SDate[1], '\0'};
                        char day2[3] = {EDate[0], EDate[1], '\0'};
                        int startDateInt = atoi(year1) * 10000 + atoi(month1) * 100 + atoi(day1);
                        int endDateInt = atoi(year2) * 10000 + atoi(month2) * 100 + atoi(day2);
                        // Validations of input
                        if (strlen(trialID) == 0 || strlen(leadDr) == 0 || strlen(partCount) == 0 || strlen(disease) == 0 || strlen(SDate) == 0 || strlen(EDate) == 0)
                        {
                            MessageBox(hwnd, "Fill in all fields!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if(!IsNum(trialID))
                        {
                            MessageBox(hwnd, "ID can only be numerical!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!OnlyAlphaChars(leadDr) || !OnlyAlphaChars(disease))
                        {
                            MessageBox(hwnd, "Mistake in either doctor or disease name!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (strlen(partCount) > 6 || !IsNum(partCount) || atoi(partCount) <= 0)
                        {
                            MessageBox(hwnd, "Enter valid participant count!\n(6 digit limit, numeric only, greater than 0)", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!ValidateDate(SDate) || !ValidateDate(EDate) || startDateInt > endDateInt)
                        {
                            MessageBox(hwnd, "Check dates!\nValid date format: DD/MM/YYYY", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else
                        {
                            // Open db and statement handlers
                            sqlite3* db;
                            sqlite3_stmt* stmt;
                            int dbstate = sqlite3_open("Trials.db", &db);
                            if (dbstate != SQLITE_OK)
                            {
                                MessageBox(hwnd, "Could not open database!", "Error", MB_OK | MB_ICONERROR);
                                break;
                            }

                            // Check if the trial with the same details exists
                            const char* checkTrialQuery = "SELECT COUNT(*) FROM Trials WHERE LeadDoctor = ? AND Disease = ? AND StartDate = ? AND EndDate = ?";
                            sqlite3_prepare_v2(db, checkTrialQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 2, disease, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 3, SDate, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 4, EDate, -1, SQLITE_STATIC);
                            sqlite3_step(stmt);
                            int found = sqlite3_column_int(stmt, 0);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);
                            sqlite3_finalize(stmt);   

                            if (found > 0)
                            {
                                // Exit the function and don't insert trial
                                MessageBox(hwnd, "This trial already exists in the database!", "Error", MB_OK | MB_ICONERROR);
                                sqlite3_close(db);
                                break;
                            }
                            
                            // Insert the trial data
                            const char* editTrialQuery = "UPDATE Trials SET LeadDoctor = ?, ParticipantCount = ?, Disease = ?, StartDate = ?, EndDate = ? WHERE TrialID = ?";
                            sqlite3_prepare_v2(db, editTrialQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, leadDr, -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 2, atoi(partCount));
                            sqlite3_bind_text(stmt, 3, disease, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 4, SDate, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 5, EDate, -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 6, atoi(trialID));
                            sqlite3_step(stmt);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);

                            // End edit and quit DB
                            sqlite3_finalize(stmt);
                            sqlite3_close(db);

                            MessageBox(hwnd, "Trial data edited successfully!", "Notice", MB_OK | MB_ICONINFORMATION);
                            SetWindowText(textboxLeadDr, "");
                            SetWindowText(textboxDis, "");
                            SetWindowText(textboxPartC, "");
                            SetWindowText(textboxSDate, "");
                            SetWindowText(textboxEDate, "");
                            SetWindowText(textboxtrialID, "");
                        }
                    }
                    else if (selectedOption == EditView_Doctors)
                    {
                        char doctorID[10] = "", drName[100] = "", specialty[100] = "";

                        GetWindowText(textboxdoctorID, doctorID, sizeof(doctorID));
                        GetWindowText(textboxDrName, drName, sizeof(drName));
                        GetWindowText(textboxSpec, specialty, sizeof(specialty));
                        // Validations of input
                        if (strlen(doctorID) == 0 || strlen(drName) == 0 || strlen(specialty) == 0)
                        {
                            MessageBox(hwnd, "Fill in all fields!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!IsNum(doctorID))
                        {
                            MessageBox(hwnd, "ID can only be numerical!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else if (!OnlyAlphaChars(drName) || !OnlyAlphaChars(specialty))
                        {
                            MessageBox(hwnd, "Mistake in either doctor or specialty name!", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }
                        else
                        {
                            // Open db and statement handlers
                            sqlite3* db;
                            sqlite3_stmt* stmt;
                            int dbstate = sqlite3_open("Trials.db", &db);
                            if (dbstate != SQLITE_OK)
                            {
                                MessageBox(hwnd, "Could not open database!", "Error", MB_OK | MB_ICONERROR);
                            }

                            // Check if a doctor with the same details exists
                            const char* checkDoctorQuery = "SELECT COUNT(*) FROM Doctors WHERE DoctorName = ? AND Specialty = ?";
                            sqlite3_prepare_v2(db, checkDoctorQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, drName, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 2, specialty, -1, SQLITE_STATIC);
                            sqlite3_step(stmt);
                            int found = sqlite3_column_int(stmt, 0);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);
                            sqlite3_finalize(stmt);
                            
                            if (found > 0)
                            {
                                // Exit the function and don't insert doctor
                                MessageBox(hwnd, "This doctor already exists in the database!", "Error", MB_OK | MB_ICONERROR);
                                sqlite3_close(db);
                                break;
                            }

                            const char* editDoctorQuery = "UPDATE Doctors SET DoctorName = ?, Specialty = ? WHERE DoctorID = ?";
                            sqlite3_prepare_v2(db, editDoctorQuery, -1, &stmt, NULL);
                            sqlite3_bind_text(stmt, 1, drName, -1, SQLITE_STATIC);
                            sqlite3_bind_text(stmt, 2, specialty, -1, SQLITE_STATIC);
                            sqlite3_bind_int(stmt, 3, atoi(doctorID));
                            sqlite3_step(stmt);
                            sqlite3_reset(stmt);
                            sqlite3_clear_bindings(stmt);
                        
                            // End edit and quit DB
                            sqlite3_finalize(stmt);
                            sqlite3_close(db);

                            MessageBox(hwnd, "Doctor data edited successfully!", "Notice", MB_OK | MB_ICONINFORMATION);
                            SetWindowText(textboxdoctorID, "");
                            SetWindowText(textboxDrName, "");
                            SetWindowText(textboxSpec, "");
                        }
                    }
                break;
            }  
        default:
            return DefWindowProc(hwnd, msg, wPar, lPar);
    }
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prevhInst, LPSTR lpStr, int show)
{
    // Define window class
    const char *className = "Database GUI Manager";
    WNDCLASS window = {0};

    // Assings the window procedure handler for handling events/actions
    window.lpfnWndProc = WindowProc;
    window.hInstance = hInst;
    window.lpszClassName = className;
    // Create background for window
    brush = CreateSolidBrush(RGB(69, 210 ,245));
    window.hbrBackground = brush;

    if(!RegisterClass(&window))
    {
        MessageBox(NULL, "Could not initialize window class! :(", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(0, className, "Clinical Trials DB Manager", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 950, 400, NULL, NULL, hInst, NULL);
    if (!hwnd)
    {
        DWORD error = GetLastError();
        char errorMsg[100];
        snprintf(errorMsg, sizeof(errorMsg), "Could not create window! :( Error: %ld", error);
        MessageBox(NULL, errorMsg, "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Create the menu on top of the window
    CreateMenuBar(hwnd);

    // Create GUI elements in the window
    lblhint1 = CreateLabel("Here the fields for inserting or editing\nwill be displayed when selecting\n\"Insert New...\" or \"Edit and View...\"", 50, 100, 50, 350, hwnd, hInst);

    lblhint2 = CreateLabel("Here the data tables will be displayed\nonly when selecting \"Edit and View...\".\nTo refresh a table after changes click\nthe current option again", 500, 100, 70, 350, hwnd, hInst);

    // Create GUI elements for Insert New... -> Trial option
    lbltrialID = CreateLabel("Trial ID", 50, 10, 20, 80, hwnd, hInst);
    textboxtrialID = CreateTextBox(100, 10, 20, 40, hwnd, hInst, TextBox_TrialID);

    lbldoctorID = CreateLabel("Doctor ID", 50, 10, 20, 80, hwnd, hInst);
    textboxdoctorID = CreateTextBox(110, 10, 20, 40, hwnd, hInst, TextBox_DoctorID);

    lblLeadDr = CreateLabel("Lead Doctor's Name", 140, 30, 20, 150, hwnd, hInst);
    textboxLeadDr = CreateTextBox(50, 50, 20, 300, hwnd, hInst, TextBox_LeadDr);

    lblPartC = CreateLabel("Number of participants", 130, 80, 20, 150, hwnd, hInst);
    textboxPartC = CreateTextBox(50, 100, 20, 300, hwnd, hInst, TextBox_ParticipantCount);

    lblDis = CreateLabel("Disease tested", 155, 130, 20, 150, hwnd, hInst);
    textboxDis = CreateTextBox(50, 150, 20, 300, hwnd, hInst, TextBox_Disease);

    lblSDate = CreateLabel("Start Date", 170, 180, 20, 150, hwnd, hInst);
    textboxSDate = CreateTextBox(50, 200, 20, 300, hwnd, hInst, TextBox_StartDate);

    lblEDate = CreateLabel("End Date", 174, 230, 20, 150, hwnd, hInst);
    textboxEDate = CreateTextBox(50, 250, 20, 300, hwnd, hInst, TextBox_EndDate);

    // Create GUI elements for Insert New... -> Doctor option
    lblDrName = CreateLabel("Doctor's Name", 150, 30, 20, 150, hwnd, hInst);
    textboxDrName = CreateTextBox(50, 50, 20, 300, hwnd, hInst, TextBox_DrName);

    lblSpec = CreateLabel("Doctor's Specialty", 140, 80, 20, 150, hwnd, hInst);
    textboxSpec = CreateTextBox(50, 100, 20, 300, hwnd, hInst, TextBox_Specialty);

    // Create data insertion button
    btnInsert = CreateButton("Confirm Insert", 150, 290, 20, 100, hwnd, hInst, Button_Insert);
    btnEdit = CreateButton("Confirm Edit", 150, 290, 20, 100, hwnd, hInst, Button_Edit);

    HideElements();

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Prevent brush memory leak
    DeleteObject(brush);

    return (int)msg.wParam;
}

// Function to summarize textbox creation
HWND CreateTextBox(int x, int y, int height, int width, HWND hwnd, HINSTANCE hInst, int textboxID)
{
    int txtBoxH = height;
    int txtBoxW = width;
    int xCoords = x;
    int yCoords = y;
    HWND textbox = CreateWindow("EDIT", "", WS_TABSTOP | WS_VISIBLE | WS_BORDER | WS_CHILD, xCoords, yCoords, txtBoxW, txtBoxH, hwnd, (HMENU)(LONG_PTR)textboxID, hInst, NULL);
    SetFont(textbox, 18, FW_NORMAL, "Arial");
    return textbox;
}

// Function to summarize button creation
HWND CreateButton(const char* btnTitle, int x, int y, int height, int width, HWND hwnd, HINSTANCE hInst, int buttonID)
{
    int btnH = height;
    int btnW = width;
    int xCoords = x;
    int yCoords = y;
    HWND button = CreateWindow("BUTTON", btnTitle, WS_TABSTOP | WS_VISIBLE | WS_BORDER | WS_CHILD, xCoords, yCoords, btnW, btnH, hwnd, (HMENU)(LONG_PTR)buttonID, hInst, NULL);
    SetFont(button, 16, FW_MEDIUM, "Arial");
    return button;
}

// Function to summarize label creation
HWND CreateLabel(const char* lblTitle, int x, int y, int height, int width, HWND hwnd, HINSTANCE hInst)
{
    int btnH = height;
    int btnW = width;
    int xCoords = x;
    int yCoords = y;
    HWND label = CreateWindow("STATIC", lblTitle, WS_VISIBLE | WS_CHILD, xCoords, yCoords, btnW, btnH, hwnd, NULL, hInst, NULL);
    SetFont(label, 16, FW_BOLD, "Arial");
    return label;
}
// Function to summarize menu creation
void CreateMenuBar(HWND hwnd)
{
    // Create menus for easier navigation
    menuBar = CreateMenu();
    insertMenu = CreateMenu();
    viewMenu = CreateMenu();

    // Create the submenus for Insert New...
    AppendMenu(insertMenu, MF_STRING, Insert_Into_Trials, "Trial");
    AppendMenu(insertMenu, MF_STRING, Insert_Into_Doctors, "Doctor");

    // Create submenus for View...
    AppendMenu(viewMenu, MF_STRING, EditView_Trials, "Trials");
    AppendMenu(viewMenu, MF_STRING, EditView_Doctors, "Doctors");

    // Add menus to the menubar
    AppendMenu(menuBar, MF_POPUP, (UINT_PTR)insertMenu, "Insert New...");
    AppendMenu(menuBar, MF_POPUP, (UINT_PTR)viewMenu, "Edit and View...");

    SetMenu(hwnd, menuBar);
}

// Functions to summarize listview (table) creation for SQL data
HWND ViewTrials(HWND hwnd, int colwidth)
{
    HWND listview = CreateWindow(WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT, 400, 10, 550, 320, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);
    ListView_SetExtendedListViewStyle(listview, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMN column = {0};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    column.pszText = "ID";
    column.cx = colwidth / 2; 
    ListView_InsertColumn(listview, 0, &column);

    column.pszText = "Lead Doctor";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 1, &column);

    column.pszText = "Participant Count";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 2, &column);

    column.pszText = "Disease";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 3, &column);

    column.pszText = "Start Date";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 4, &column);

    column.pszText = "End Date";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 5, &column);

    sqlite3* db;
    sqlite3_stmt* stmt;
    int dbstate = sqlite3_open("Trials.db", &db);
    if (dbstate != SQLITE_OK)
    {
        MessageBox(hwnd, "Could not open database!", "Error", MB_OK | MB_ICONERROR);
        return listview;
    }

    const char* selectTrials = "SELECT TrialID, LeadDoctor, ParticipantCount, Disease, StartDate, EndDate FROM Trials";
    sqlite3_prepare_v2(db, selectTrials, -1, &stmt, NULL);

    int row = 0;  // Number of rows (records) to display 
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        // Insert the new row in listview
        LVITEM data = {0};
        data.mask = LVIF_TEXT;
        data.iItem = row;
        data.iSubItem = 0;
        data.pszText = (char*)sqlite3_column_text(stmt, 0);
        if (data.pszText == NULL)
        {
            data.pszText = "NULL";  // Handle NULL fields
        } 
        ListView_InsertItem(listview, &data); //Insert first column in row

        // Insert subsequent columns safely
        for (int col = 1; col <= 5; col++) 
        {
            const unsigned char* colData = sqlite3_column_text(stmt, col);
            ListView_SetItemText(listview, row, col, colData ? (char*)colData : "NULL");
        }

        row++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    UpdateWindow(listview);
    return listview;
}

HWND ViewDoctors(HWND hwnd, int colwidth)
{
    HWND listview = CreateWindow(WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 400, 10, 250, 320, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
    ListView_SetExtendedListViewStyle(listview, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMN column = {0};
    column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    column.pszText = "ID";
    column.cx = colwidth / 2; 
    ListView_InsertColumn(listview, 0, &column);

    column.pszText = "Doctor Name";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 1, &column);

    column.pszText = "Specialty";
    column.cx = colwidth;
    ListView_InsertColumn(listview, 2, &column);

    sqlite3* db;
    sqlite3_stmt* stmt;
    int dbstate = sqlite3_open("Trials.db", &db);
    if (dbstate != SQLITE_OK)
    {
        MessageBox(hwnd, "Could not open database!", "Error", MB_OK | MB_ICONERROR);
        return listview;
    }

    const char* selectDoctors = "SELECT DoctorID, DoctorName, Specialty FROM Doctors";
    sqlite3_prepare_v2(db, selectDoctors, -1, &stmt, NULL);

    int row = 0;  // Number of rows (records) to display 
    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        // Insert the new row in listview
        LVITEM data = {0};
        data.mask = LVIF_TEXT;
        data.iItem = row;
        data.iSubItem = 0;
        data.pszText = (char*)sqlite3_column_text(stmt, 0);

        if (data.pszText == NULL)
        {
            data.pszText = "NULL";  // Handle NULL fields
        } 
        ListView_InsertItem(listview, &data); //Insert first column in row

        // Insert subsequent columns safely
        for (int col = 1; col <= 3; col++) 
        {
            const unsigned char* colData = sqlite3_column_text(stmt, col);
            ListView_SetItemText(listview, row, col, colData ? (char*)colData : "NULL");
        }

        row++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    UpdateWindow(listview);
    return listview;
}

// Function to set font to different elements
void SetFont(HWND element, int fontSize, int fontWeight, const char* name)
{
    HFONT font = CreateFont(fontSize, 0, 0, 0, fontWeight, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, name);
    SendMessage(element, WM_SETFONT, (WPARAM)font, TRUE);
}

// Hide all elements initially at start of program
void HideElements(void)
{
    ShowWindow(lblLeadDr, SW_HIDE);
    ShowWindow(lblPartC, SW_HIDE);
    ShowWindow(lblDis, SW_HIDE);
    ShowWindow(lblSDate, SW_HIDE);
    ShowWindow(lblEDate, SW_HIDE);
    ShowWindow(textboxLeadDr, SW_HIDE);
    ShowWindow(textboxPartC, SW_HIDE);
    ShowWindow(textboxDis, SW_HIDE);
    ShowWindow(textboxSDate, SW_HIDE);
    ShowWindow(textboxEDate, SW_HIDE);

    ShowWindow(lblDrName, SW_HIDE);
    ShowWindow(lblSpec, SW_HIDE);
    ShowWindow(textboxDrName, SW_HIDE);
    ShowWindow(textboxSpec, SW_HIDE);

    ShowWindow(btnInsert, SW_HIDE);
    ShowWindow(btnEdit, SW_HIDE);

    ShowWindow(lbltrialID, SW_HIDE);
    ShowWindow(textboxtrialID, SW_HIDE);
    ShowWindow(lbldoctorID, SW_HIDE);
    ShowWindow(textboxdoctorID, SW_HIDE);
}

bool IsNum(const char* str) 
{
    for (int i = 0; str[i] != '\0'; i++) 
    {
        if (!isdigit(str[i])) 
        {
            return false; // If any character is not a digit 
        }
    }
    return true; // If all characters are digits
}

bool OnlyAlphaChars(const char *str)
{
    for (int i = 0; str[i] != '\0'; i++) 
    {
        if (!isalpha(str[i]) && !isspace(str[i])) 
        {
            return false; // If any character is not from the ABC or not a space
        }
    }
    return true; // If all characters are from the ABC
}

// Check if leap year
bool IsLeapYear(int year) 
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Validate date format DD/MM/YYYY
bool ValidateDate(const char* date) 
{
    // Check if date has correct length
    if (strlen(date) != 10) 
    {
        return false;
    }
    // Check if separators are in the right positions
    if (date[2] != '/' || date[5] != '/')
    {
        return false;
    }
    // Get day, month, and year as integers
    char strDay[3] = {date[0], date[1], '\0'};
    char strMonth[3] = {date[3], date[4], '\0'};
    char strYear[5] = {date[6], date[7], date[8], date[9], '\0'};

    if (!isdigit(strDay[0]) || !isdigit(strDay[1]) || !isdigit(strMonth[0]) || !isdigit(strMonth[1]) || !isdigit(strYear[0]) || !isdigit(strYear[1]) || !isdigit(strYear[2]) || !isdigit(strYear[3])) 
    {
        return false;
    }
    
    int day = atoi(strDay);
    int month = atoi(strMonth);
    int year = atoi(strYear);

    // Validate ranges for day, month, and year
    if (month < 1 || month > 12)
    {
        return false;
    } 

    if (year < 1950 || year > 9999)
    {
        return false;
    }

    
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Make February have 29 for leap years
    if (month == 2 && IsLeapYear(year)) 
    {
        daysInMonth[1] = 29;
    }

    // Validate day range
    if (day < 1 || day > daysInMonth[month - 1]) 
    {
        return false;
    }

    return true;
}