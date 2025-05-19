USE ChatAppDB
GO

--------------------------------------------------------------------------------------------------------------------------------------------

IF OBJECT_ID('dbo.AddUser', 'P') IS NOT NULL
BEGIN
	DROP PROCEDURE AddUser;
END;
GO

CREATE PROCEDURE AddUser
	@UserName NVARCHAR(50),
	@PasswordHash NVARCHAR(256), 
	@PhoneNumber VARCHAR(15)

AS BEGIN

	IF EXISTS (SELECT 1 FROM Users WHERE PhoneNumber = @PhoneNumber)
		RETURN -1;

	IF @PhoneNumber LIKE '%[^0-9]%'
		RETURN -2;

	INSERT INTO Users (Username, PasswordHash, PhoneNumber) 
	VALUES 
	(@UserName, @PasswordHash,  @PhoneNumber);

	DECLARE @habibi INT;
	SELECT @habibi = UserID FROM Users WHERE PhoneNumber = @PhoneNumber;
	RETURN @habibi;

END;
GO

--------------------------------------------------------------------------------------------------------------------------------------------

IF OBJECT_ID('dbo.DeleteUser', 'P') IS NOT NULL
BEGIN 
	DROP PROCEDURE DeleteUser;
END;
GO

CREATE PROCEDURE DeleteUser
	@PhoneNumber VARCHAR(15),
	@UserID INT = NULL

AS BEGIN
	
	SELECT @UserID = UserID 
	FROM Users 
	WHERE PhoneNumber = @PhoneNumber;

	DELETE FROM Contacts WHERE ContactID = @UserID; 
	DELETE FROM Users WHERE UserID = @UserID;

END;
GO

--------------------------------------------------------------------------------------------------------------------------------------------

IF OBJECT_ID('dbo.ShowContacts', 'P') IS NOT NULL
BEGIN 
	DROP PROCEDURE ShowContacts;
END;
GO

CREATE PROCEDURE ShowContacts
	@UserID INT

AS BEGIN
	
	SELECT u2.UserID, u2.Username, u2.PhoneNumber, c.ConversationFile
	FROM Users u
	INNER JOIN Contacts c ON u.UserID = c.UserID
	INNER JOIN Users u2 ON c.ContactID = u2.UserID
	WHERE u.UserID = @UserID;

END;
GO

------------------------------------------------------------------------------------------------------------------------------------------------
IF OBJECT_ID('dbo.AddContact', 'P') IS NOT NULL
BEGIN 
	DROP PROCEDURE AddContact;
END;
GO

CREATE PROCEDURE AddContact
    @UserID INT,
    @ContactUserID INT,
    @ReturnCode INT OUTPUT,
	@ConversationFile NVARCHAR(255)
AS
BEGIN
    IF @UserID = @ContactUserID
    BEGIN
        SET @ReturnCode = -1 
        RETURN
    END

    IF EXISTS (
        SELECT 1 FROM Contacts
        WHERE UserID = @UserID AND ContactID = @ContactUserID
    )
    BEGIN
        SET @ReturnCode = 1 
        RETURN
    END

    IF NOT EXISTS (SELECT 1 FROM Users WHERE UserID = @UserID)
        OR NOT EXISTS (SELECT 1 FROM Users WHERE UserID = @ContactUserID)
    BEGIN
        SET @ReturnCode = -2 
        RETURN
    END

    -- Inserare contact
    INSERT INTO Contacts(UserID, ContactID, ConversationFile)
    VALUES (@UserID, @ContactUserID, @ConversationFile)

    SET @ReturnCode = 0 -- Succes
END
GO
------------------------------------------------------------------------------------------------------------------------------------------------

IF OBJECT_ID('dbo.DeleteContact', 'P') IS NOT NULL
BEGIN 
	DROP PROCEDURE DeleteContact;
END;
GO

CREATE PROCEDURE DeleteContact
    @UserID INT,
    @ContactUserID INT
AS
BEGIN

    IF NOT EXISTS (SELECT 1 FROM Contacts WHERE UserID = @UserID AND ContactID = @ContactUserID)
		RETURN -1;

	DELETE Contacts WHERE UserID = @UserID AND ContactID = @ContactUserID;

	RETURN 0;
END
GO

------------------------------------------------------------------------------------------------------------------------------------------------
IF OBJECT_ID('dbo.GetContactIDByPhone', 'P') IS NOT NULL
BEGIN 
	DROP PROCEDURE GetContactIDByPhone;
END;
GO


CREATE PROCEDURE GetContactIDByPhone
    @UserID INT,
    @PhoneNumber NVARCHAR(20),
    @ContactID INT OUTPUT
AS
BEGIN
    SET NOCOUNT ON;

    DECLARE @FoundContactID INT;

    SELECT @FoundContactID = UserID
    FROM Users
    WHERE PhoneNumber = @PhoneNumber;

    IF @FoundContactID IS NULL
    BEGIN
        SET @ContactID = -1;
        RETURN;
    END

    IF EXISTS (
        SELECT 1
        FROM Contacts
        WHERE UserID = @UserID AND ContactID = @FoundContactID
    )
    BEGIN
        SET @ContactID = @FoundContactID;
    END
    ELSE
    BEGIN
        SET @ContactID = -1;
    END
END;
GO
