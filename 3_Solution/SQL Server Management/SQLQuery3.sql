USE ChatAppDB;

IF OBJECT_ID('dbo.AddUser', 'P') IS NOT NULL
BEGIN
	EXEC AddUser @UserName='viku', @PasswordHash='CWA78uy$ge5s', @PhoneNumber='4965694409';
END;


IF OBJECT_ID('dbo.DeleteUser', 'P') IS NOT NULL
BEGIN 
	EXEC DeleteUser @PhoneNumber='5';
END;


IF OBJECT_ID('dbo.ShowContacts', 'P') IS NOT NULL
BEGIN 
	EXEC ShowContacts @UserID = 1;
END;

DECLARE @Result INT;

EXEC GetContactIDByPhone 
    @UserID = 1, 
    @PhoneNumber = '9876543210', 
    @ContactID = @Result OUTPUT;

SELECT @Result AS ContactID;
