CREATE TABLE Doctors (
    DoctorID INTEGER PRIMARY KEY AUTOINCREMENT, 
    DoctorName TEXT NOT NULL UNIQUE,           
    Specialty TEXT                             
);

CREATE TABLE Trials (
    TrialID INTEGER PRIMARY KEY AUTOINCREMENT, 
    LeadDoctor TEXT NOT NULL,                  
    ParticipantCount INTEGER NOT NULL CHECK (ParticipantCount > 0), 
    Disease TEXT NOT NULL,                     
    StartDate TEXT NOT NULL,                   
    EndDate TEXT NOT NULL,                     
    FOREIGN KEY (LeadDoctor) REFERENCES Doctors (DoctorName) 
    ON DELETE RESTRICT                
    ON UPDATE CASCADE                         
);

CREATE TRIGGER UpdateDoctorName
AFTER UPDATE OF LeadDoctor ON Trials
BEGIN
    UPDATE Doctors
    SET DoctorName = NEW.LeadDoctor
    WHERE DoctorName = OLD.LeadDoctor;
END;

CREATE TRIGGER UpdateLeadDoctorName
AFTER UPDATE ON Doctors
FOR EACH ROW
BEGIN
   UPDATE Trials 
   SET LeadDoctor = NEW.DoctorName
   WHERE LeadDoctor = OLD.DoctorName;
END;