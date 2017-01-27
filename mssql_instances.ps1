function getInstances
{
    $services = Get-WmiObject win32_service | where {$_.name -like "MSSQL*"}
    $instances = @()
    foreach ($service in $services)
    {
        if (($service.name -eq "MSSQLSERVER") -or ($service.name -like "MSSQL$*"))
        {
          if($service.name -eq "MSSQLSERVER"){
            $instances += "SQLServer"
          }else{
            $instances += $service.name
          }
        }
    }
    return $instances
}

$instances = getInstances
Write-Host $instances 
