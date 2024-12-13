import logo from './logo.svg';
import './App.css';
import 'bootstrap/dist/css/bootstrap.min.css';
import 'react-toastify/dist/ReactToastify.css';
import {Container} from 'react-bootstrap';
import Login from './Components/Login';
import Home from './Components/Home';
import {BrowserRouter as Router, Route, Routes} from 'react-router-dom';
import ContextProvider from './ContexApi';
import {ToastContainer} from 'react-toastify';
import { useContextApi } from './ContexApi';
import { useEffect } from 'react';
import axios from 'axios';
function App() {
  const { userData, setUserData, token, setToken, setLogin } = useContextApi();
  useEffect(()=>{
    let tempTok = localStorage.getItem("token");
    setLogin(localStorage.getItem("token") != null);
    if (tempTok != null) {
        axios.defaults.headers.common['Authorization'] = `Bearer ${tempTok}`;
        axios.get("http://localhost:2028/api/user/").then((res) => {
            if (res.status == 200) {
                setUserData(res.data);
                setLogin(true);
                setToken(tempTok);
            }
        }).catch((error)=>{
          setLogin(false);
          localStorage.removeItem("token");
          setToken(null);
          setUserData(null);
          console.log(error);
        });
    }

  }, [])
  return (
      <>
        <ToastContainer />
        <Router>
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/login" element={<Login />} />
          </Routes>
        </Router>
      </>
  );
}

export default App;
